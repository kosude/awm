// htable.h - Hash table implementation
// (C) 2019 Srimanta Barua <srimanta.barua1@gmail.com>


#ifndef __TCWM_HTABLE_H__
#define __TCWM_HTABLE_H__


#include <stdint.h>


// Hash table errors
enum htable_err {
	HTE_OK,    // No problems
	HTE_NOKEY, // Key not found (for get() and pop())
	HTE_EXIST  // Key already exists (for set())
};


// -------- uint32_t-keyed hash table --------

// uint32_t-keyed hash table. This is an opaque handle to the hash table
struct htable_u32;

            // (modified for the bwm project)
typedef struct htable_u32 htable_u32_t;
typedef enum htable_err htable_err_t;

// Allocate a new hash table
struct htable_u32* htable_u32_new(void);

// Free hash table. If "free_cb" is not NULL, use it to free all values
void htable_u32_free(struct htable_u32 *ht, void (*free_cb) (void*));

// Does value exist in hash table?
uint8_t htable_u32_contains(const struct htable_u32 *ht, uint32_t key);

// Get value for key "key". If doesn't exist return NULL, and if "err" is not NULL place error
// code in "err"
void* htable_u32_get(struct htable_u32 *ht, uint32_t key, enum htable_err *err);

// Set value for key "key". Return status code
enum htable_err htable_u32_set(struct htable_u32 *ht, uint32_t key, void *val);

// Delete and r eturn value for key "key". If doesn't exist return NULL, and if "err" is not
// NULL place error code in "err"
void* htable_u32_pop(struct htable_u32 *ht, uint32_t key, enum htable_err *err);


#endif // __TCWM_HTABLE_H__
