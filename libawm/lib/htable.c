// htable.c - Implementation of hash tables
// (C) 2019 Srimanta Barua <srimanta.barua1@gmail.com>


#include <stdlib.h>
#include <stdio.h>

#include "libawm/htable.h"


// -------- uint32_t-keyed hash table ----------------

// Block size for resizing
#define HTU32_BLK 32

// Hash table node
struct node_u32 {
	void     *val;   // Value
	uint32_t key;    // Key
	uint8_t  is_occ; // Is this slot occupied?
};

// uint32_t-keyed hash table
struct htable_u32 {
	uint32_t        cap;    // Max capacity of table
	uint32_t        size;   // Size of table
	struct node_u32 *nodes; // Array of nodes
};


// Get hash for uint32_t key
// Copied shamelessly from https://github.com/skeeto/hash-prospector - lowbias32()
static uint32_t _hash_u32_u32(uint32_t key) {
	key ^= key >> 16;
	key *= 0x7feb352d;
	key ^= key >> 15;
	key *= 0x846ca68b;
	key ^= key >> 16;
	return key;
}


// Allocate a new hash table
struct htable_u32* htable_u32_new(void) {
	struct htable_u32 *ht;
	if (!(ht = calloc(1, sizeof(struct htable_u32)))) {
        printf("calloc()");
        exit(1);
	}
	return ht;
}


// Free hash table. If "free_cb" is not NULL, use it to free all values
void htable_u32_free(struct htable_u32 *ht, void (*free_cb) (void*)) {
	uint32_t i;
	if (!ht) {
        printf("NULL htable_u32");
        exit(1);
	}
	if (free_cb) {
		for (i = 0;  i < ht->cap; i++) {
			if (ht->nodes[i].val && ht->nodes[i].is_occ) {
				free_cb(ht->nodes[i].val);
			}
		}
	}
	free(ht->nodes);
	free(ht);
}


// Does value exist in hash table?
uint8_t htable_u32_contains(const struct htable_u32 *ht, uint32_t key) {
	uint32_t i;
	if (!ht) {
        printf("NULL htable_u32");
        exit(1);
	}
	if (!ht->cap || !ht->size) {
		return 0;
	}
	i = _hash_u32_u32(key) % ht->cap;
	while (ht->nodes[i].is_occ) {
		if (ht->nodes[i].key == key) {
			return 1;
		}
		i = (i + 1) % ht->cap;
	}
	return 0;
}


// Get value for key "key". If doesn't exist return NULL, and if "err" is not NULL place error
// code in "err"
void* htable_u32_get(struct htable_u32 *ht, uint32_t key, enum htable_err *err) {
	uint32_t i;
	if (!ht) {
        printf("NULL htable_u32");
        exit(1);
	}
	if (!ht->cap || !ht->size) {
		if (err) {
			*err = HTE_NOKEY;
		}
		return NULL;
	}
	i = _hash_u32_u32(key) % ht->cap;
	while (ht->nodes[i].is_occ) {
		if (ht->nodes[i].key == key) {
			if (err) {
				*err = HTE_OK;
			}
			return ht->nodes[i].val;
		}
		i = (i + 1) % ht->cap;
	}
	if (err) {
		*err = HTE_NOKEY;
	}
	return NULL;
}


// Set value for key "key". Return status code
enum htable_err htable_u32_set(struct htable_u32 *ht, uint32_t key, void *val) {
	uint32_t i, j;
	struct node_u32 *tmp;
	if (!ht) {
        printf("NULL htable_u32");
        exit(1);
	}
	// Is table too small?
	if (ht->cap <= ht->size << 1) {
		// Allocate new table
		if (!(tmp = calloc(ht->cap + HTU32_BLK, sizeof(struct node_u32)))) {
            printf("calloc()");
            exit(1);
		}
		// Rehash
		for (i = 0; i < ht->cap; i++) {
			if (!ht->nodes[i].is_occ) {
				continue;
			}
			j = _hash_u32_u32(ht->nodes[i].key) % (ht->cap + HTU32_BLK);
			while (tmp[j].is_occ) {
				j = (j + 1) % (ht->cap + HTU32_BLK);
			}
			tmp[j] = ht->nodes[i];
		}
		// Cleanup
		free(ht->nodes);
		ht->nodes = tmp;
		ht->cap += HTU32_BLK;
	}
	// Hash new key
	i = _hash_u32_u32(key) % ht->cap;
	while (ht->nodes[i].is_occ) {
		if (ht->nodes[i].key == key) {
			return HTE_EXIST;
		}
		i = (i + 1) % ht->cap;
	}
	ht->nodes[i].key = key;
	ht->nodes[i].val = val;
	ht->nodes[i].is_occ = 1;
	ht->size++;
	return HTE_OK;
}


// Delete and r eturn value for key "key". If doesn't exist return NULL, and if "err" is not
// NULL place error code in "err"
void* htable_u32_pop(struct htable_u32 *ht, uint32_t key, enum htable_err *err) {
	uint32_t i;
	if (!ht) {
        printf("NULL htable_u32");
        exit(1);
	}
	if (!ht->cap || !ht->size) {
		if (err) {
			*err = HTE_NOKEY;
		}
		return NULL;
	}
	i = _hash_u32_u32(key) % ht->cap;
	while (ht->nodes[i].is_occ) {
		if (ht->nodes[i].key == key) {
			if (err) {
				*err = HTE_OK;
			}
			ht->nodes[i].is_occ = 0;
			ht->size--;
			return ht->nodes[i].val;
		}
		i = (i + 1) % ht->cap;
	}
	if (err) {
		*err = HTE_NOKEY;
	}
	return NULL;
}
