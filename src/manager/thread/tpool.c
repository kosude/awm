/*
 *   Copyright (c) 2023 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "tpool.h"

#include <stdlib.h>
#include <pthread.h>

#include <stdio.h>

/**
 * A work struct used to queue the work for a thread in a singly linked list.
 */
typedef struct tpool_work_t {
    /** The function to be called by this work item. */
    tpool_func_t func;
    /** Single argument passed to `func`. */
    void *arg;

    /** The next work item in the linked list. */
    struct tpool_work_t *next;
} tpool_work_t;

/**
 * Initialise data for a new work object.
 */
static tpool_work_t *tpool_work_init(
    const tpool_func_t func,
    void *const arg
);

/**
 * Deallocate memory reserved for the given work data object.
 */
static void tpool_work_dealloc(
    tpool_work_t *const work
);

/**
 * Pull the first work item in the given thread pool's queue, removing it from the list.
 */
static tpool_work_t *tpool_work_pull(
    tpool_t *const pool
);

/**
 * Worker function called by threads on creation within the thread pool.
 *
 * `arg` is assumed to be the thread pool.
 */
static void *const tpool_worker_proc(
    void *const arg
);

typedef struct tpool_t {
    /** First work item in the list. */
    tpool_work_t *whead;
    /** Last work item in the list. */
    tpool_work_t *wtail;

    /** Mutex used for work locking. */
    pthread_mutex_t wmutex;

    /** Conditional signals that there is work to be processed. */
    pthread_cond_t work_exists;
    /** Conditional which will signal to tpool_wait(). */
    pthread_cond_t all_done;

    /** Amount of threads actively processing work. */
    uint32_t workingn;
    /** Amount of alive threads in the pool. */
    uint32_t threadn;

    /** Boolean, starts at 0 and is set to 1 when any jobs are added. */
    uint8_t used;
    /** Boolean, set to true to stop the threads. */
    uint8_t stop;
} tpool_t;

tpool_t *tpool_init(const uint32_t n) {
    tpool_t *pool;
    pthread_t tid;

    uint32_t cn = (n > 0) ? n : 2; // clamp n to minimum

    pool = malloc(sizeof(tpool_t));

    pool->whead = NULL;
    pool->wtail = NULL;

    pthread_mutex_init(&(pool->wmutex), NULL);
    pthread_cond_init(&(pool->work_exists), NULL);
    pthread_cond_init(&(pool->all_done), NULL);

    pool->workingn = 0;
    pool->threadn = cn;

    pool->used = 0;
    pool->stop = 0;

    for (uint32_t i = 0; i < cn; i++) {
        // create thread running the tpool_worker_proc() worker function
        pthread_create(&tid, NULL, tpool_worker_proc, pool);

        // threads are detached to be cleaned up on exit
        pthread_detach(tid);
    }

    return pool;
}

void tpool_dealloc(tpool_t *const pool) {
    tpool_work_t *wcur, *wnext;

    pthread_mutex_lock(&(pool->wmutex));

    // destroy every work item in the list
    // note that anything currently processing won't be interrupted since those threads pulled the work they're doing out of the list
    wcur = pool->whead;
    while (wcur) {
        wnext = wcur->next;

        tpool_work_dealloc(wcur);
        wcur = wnext;
    }

    // tell threads to stop
    pool->stop = 1;
    pthread_cond_broadcast(&(pool->work_exists));

    pthread_mutex_unlock(&(pool->wmutex));

    // wait on all threads to finish before destroying scheduling primitives
    tpool_wait(pool);

    // destroy scheduling primitives
    pthread_mutex_destroy(&(pool->wmutex));
    pthread_cond_destroy(&(pool->work_exists));
    pthread_cond_destroy(&(pool->all_done));

    free(pool);
}

uint8_t tpool_add_work(tpool_t *const pool, const tpool_func_t func, void *const arg) {
    tpool_work_t *work = tpool_work_init(func, arg);
    if (!work) {
        return 0;
    }

    pthread_mutex_lock(&(pool->wmutex));

    // add work to the end of the queue
    if (!pool->whead) {
        pool->whead = work;
        pool->wtail = pool->whead;
    } else {
        pool->wtail->next = work;
        pool->wtail = work;
    }

    pool->used = 1;

    pthread_cond_broadcast(&(pool->work_exists));

    pthread_mutex_unlock(&(pool->wmutex));

    return 1;
}

void tpool_wait(tpool_t *const pool) {
    printf("Waiting...\n");

    pthread_mutex_lock(&(pool->wmutex));

    // wait until the pool is fully stopped
    for (;;) {
        if (pool->used && ((!pool->stop && pool->workingn > 0) || (pool->threadn > 0))) {
            pthread_cond_wait(&(pool->all_done), &(pool->wmutex));
        }

        break;
    }

    // thread pool is stopped, reset used to 0 for reuse (otherwise waiting for a pool to stop twice results in hangs)
    pool->used = 0;

    pthread_mutex_unlock(&(pool->wmutex));
}

static tpool_work_t *tpool_work_init(const tpool_func_t func, void *const arg) {
    tpool_work_t *work;

    work = malloc(sizeof(tpool_work_t));
    work->func = func;
    work->arg = arg;
    work->next = NULL;

    return work;
}

static void tpool_work_dealloc(tpool_work_t *const work) {
    free(work);
}

static tpool_work_t *tpool_work_pull(tpool_t *const pool) {
    tpool_work_t *work = pool->whead;

    if (!work) {
        return NULL;
    }

    // pop work from the linked list
    if (!work->next) {
        pool->whead = NULL;
        pool->wtail = NULL;
    } else {
        pool->whead = work->next;
    }

    return work;
}

static void *const tpool_worker_proc(void *const arg) {
    tpool_t *pool = arg;
    tpool_work_t *work;

    // the thread runs continuously
    for (;;) {
        pthread_mutex_lock(&(pool->wmutex));

        // this check is re-done every time the work_exists cond is signalled
        // therefore, this blocks until work is available to be done.
        // also note that wmutex is automatically unlocked until the cond is signalled so other threads can unlock it to submit work.
        while (!pool->whead && !pool->stop) {
            pthread_cond_wait(&(pool->work_exists), &(pool->wmutex));
        }

        // break loop if the pool has requested threads to stop, do this before work is pulled
        if (pool->stop) {
            break;
        }

        // pull work and increment working count
        work = tpool_work_pull(pool);
        pool->workingn++;

        // mutex is unlocked so other threads can also pull and process work
        pthread_mutex_unlock(&(pool->wmutex));

        if (work) {
            // call work function and destroy it
            work->func(work->arg);
            tpool_work_dealloc(work);
        }

        // work (if there was any) is now done, lock the mutex again to sync the pool data
        pthread_mutex_lock(&(pool->wmutex));

        // working count decreased
        pool->workingn--;

        // if there is no work left
        if (!pool->stop && pool->workingn == 0 && !pool->whead) {
            pthread_cond_signal(&(pool->all_done));
        }
        pthread_mutex_unlock(&(pool->wmutex));
    }

    // this thread is stopping
    pool->threadn--;

    pthread_cond_signal(&(pool->all_done));
    pthread_mutex_unlock(&(pool->wmutex));

    return NULL;
}
