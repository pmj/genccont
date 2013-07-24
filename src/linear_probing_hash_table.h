/*
Copyright (c) 2013 Phil Jordan <phil@philjordan.eu>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

/*
 * An implementation of a open addressing hash table with linear probing.
 * Open addressing makes generic implementation a little trickier than chaining,
 * at least without reverting to macro overuse. Nevertheless, I think this is a
 * reasonable compromise.
 *
 * Note that a realloc which disallows growing will cause problems with this
 * implementation.
 *
 * It is assumed that items can be freely copied using memcpy/memmove.
 * TODO: make copying items a pluggable function.
 */

#ifndef GENCCONT_LINEAR_PROBING_HASH_TABLE_H
#define GENCCONT_LINEAR_PROBING_HASH_TABLE_H

#include "hash_shared.h"

#if defined(KERNEL) && defined(APPLE)
/* xnu kernel */
/* xnu for some reason doesn't typedef ptrdiff_t. To avoid stepping on toes,
 * we'll temporarily re-#define it in case another header typedefs it */
#define ptrdiff_t __darwin_ptrdiff_t
#elif !defined(__KERNEL__) && !defined(KERNEL)
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


struct genc_linear_probing_hash_table;

/* Initialises the empty hash table with the given function implementations and capacity.
 * Defaults (70, 0) are used for load factor percentage thresholds for growing and shrinking. */
genc_bool_t genc_linear_probing_hash_table_init(
	struct genc_linear_probing_hash_table* table,
	genc_key_hash_fn hash_fn,
	genc_hash_get_item_key_fn get_key_fn,
	genc_hash_key_equality_fn key_equality_fn,
	genc_item_is_empty_fn item_empty_fn,
	genc_item_clear_fn item_clear_fn,
	genc_realloc_fn realloc_fn /* Make sure this fulfils the buckets' alignment requirements! */,
	void* opaque,
	size_t bucket_size, /* bytes per item */
	size_t initial_capacity_pow2);

/* initialises the hash table with non-default thresholds.
 * Table will only grow on insertion and shrink on removal.
 * Grow threshold should be somewhat more than 2x shrink threshold to avoid
 * oscillation.
 * Capacity is always a power of 2.
 */
genc_bool_t genc_linear_probing_hash_table_init_ext(
	struct genc_linear_probing_hash_table* table,
	genc_key_hash_fn hash_fn,
	genc_hash_get_item_key_fn get_key_fn,
	genc_hash_key_equality_fn key_equality_fn,
	genc_item_is_empty_fn item_empty_fn,
	genc_item_clear_fn item_clear_fn,
	genc_realloc_fn realloc_fn,
	void* opaque,
	size_t bucket_size, /* bytes per item */
	size_t initial_capacity_pow2,
	uint8_t load_percent_grow_threshold,
	uint8_t load_percent_shrink_threshold);

/* Returns the current number of items in the hash table. */
size_t genc_lpht_count(struct genc_linear_probing_hash_table* table);

/* Returns the number of buckets allocated. */
size_t genc_lpht_capacity(struct genc_linear_probing_hash_table* table);

/* Drops all items from the table and deallocates bucket array memory. */
void genc_lpht_destroy(struct genc_linear_probing_hash_table* table);

/* Inserts the given item into the hash table.
 * Returns NULL to report failure due to a duplicate or growth failure, pointer
 * to inserted bucket on success. */
void* genc_lpht_insert_item(
	struct genc_linear_probing_hash_table* table, void* item);

/* Looks up the key in the table, returning the matching item if present, or NULL otherwise. */
void* genc_lpht_find(struct genc_linear_probing_hash_table* table, void* key);

/* Hashes the key and returns the bucket index into which the key falls */
genc_hash_t genc_lpht_get_bucket_for_key(
	struct genc_linear_probing_hash_table* table, void* key);

/* Removes the item from the hash table.
 * Deallocation, like allocation, is the responsibility of the caller.
 * item must point to the location of the value within the table - i.e.
 * returned by genc_lpht_find or genc_lpht_insert_item
 */
void genc_lpht_remove(struct genc_linear_probing_hash_table* table, void* item);

/* Shrink the capacity of the table by a factor of 1 << log2_shrink_factor */
void genc_lpht_shrink_by(struct genc_linear_probing_hash_table* table, unsigned log2_shrink_factor);
/* Grow the capacity of the table by a factor of 1 << log2_grow_factor */
void genc_lpht_grow_by(struct genc_linear_probing_hash_table* table, unsigned log2_grow_factor);

/* Walks all the elements in the hash table and checks they're still in the correct bucket. */
genc_bool_t genc_lpht_verify(struct genc_linear_probing_hash_table* table);

/* Iterating over all the items in the table: */

/* First non-empty bucket. */
void* genc_lpht_first_item(struct genc_linear_probing_hash_table* table);
/* Next non-empty bucket */
void* genc_lpht_next_item(struct genc_linear_probing_hash_table* table, void* cur_item);

/* Empty all buckets in the table and reset the item count back to 0, but do not
 * free/alloc any memory. */
void genc_lpht_clear(struct genc_linear_probing_hash_table* table);


struct genc_linear_probing_hash_table
{
	genc_key_hash_fn hash_fn;
	genc_hash_get_item_key_fn get_key_fn;
	genc_hash_key_equality_fn key_equality_fn;
	genc_item_is_empty_fn item_empty_fn;
	genc_item_clear_fn item_clear_fn;
	genc_realloc_fn realloc_fn;
	void* opaque;
	size_t bucket_size; /* bytes per item */
	size_t capacity;
	size_t item_count;
	void* buckets;
	uint8_t load_percent_grow_threshold;
	uint8_t load_percent_shrink_threshold;
};
typedef struct genc_linear_probing_hash_table genc_linear_probing_hash_table_t;

#define genc_lpht_first_obj(table, type) \
GENC_CXX_CAST(type*, genc_lpht_first_item(table))

#define genc_lpht_next_obj(table, cur_obj, type) \
GENC_CXX_CAST(type*, genc_lpht_next_item(table, GENC_CXX_CAST(type*, cur_obj)))

#define genc_lpht_find_obj(table, key, type) \
GENC_CXX_CAST(type*, genc_lpht_find(table, key))

#define genc_lpht_insert_obj(table, new_obj, type) \
GENC_CXX_CAST(type*, genc_lpht_insert_item(table, GENC_CXX_CAST(type*, new_obj)))


#if defined(KERNEL) && defined(APPLE)
#undef ptrdiff_t
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
