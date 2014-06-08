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

enum genc_lpht_insertion_test_result_type
{
	// inserting a NULL item is a no-op
	GENC_LPHT_INSERT_NULL,
	// inserting is straightforward: pick a bucket, write to it
	GENC_LPHT_INSERT_SIMPLE,
	// inserting would trigger a resize (realloc array to resize_bytes) but would succeed even if resizing fails
	GENC_LPHT_INSERT_WANTS_RESIZE,
	// inserting would trigger a resize and if the realloc fails, so will the insertion
	GENC_LPHT_INSERT_NEEDS_RESIZE,
	// The table is full, but resizing would trigger an overflow.
	GENC_LPHT_INSERT_SIZE_OVERFLOW,
	// An item with the same key is already present in the table, so the insertion would fail
	GENC_LPHT_INSERT_KEY_EXISTS
};
struct genc_lpht_insertion_test_result
{
	enum genc_lpht_insertion_test_result_type type;
	size_t resize_bytes;
};
typedef struct genc_lpht_insertion_test_result genc_lpht_insertion_test_result_t;
/* "What-if" function: if we were to insert item, what would happen? */
genc_lpht_insertion_test_result_t genc_lpht_can_insert_item(
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
bool genc_lpht_shrink_by(struct genc_linear_probing_hash_table* table, unsigned log2_shrink_factor);
/* Grow the capacity of the table by a factor of 1 << log2_grow_factor */
bool genc_lpht_grow_by(struct genc_linear_probing_hash_table* table, unsigned log2_grow_factor);

bool genc_lpht_resize(struct genc_linear_probing_hash_table* table, size_t new_capacity);

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

struct genc_linear_probing_hash_table_light
{
	/* Total number of buckets */
	size_t capacity;
	/* Number of filled buckets */
	size_t item_count;
	void* buckets;
};
typedef struct genc_linear_probing_hash_table_light genc_linear_probing_hash_table_light_t;

struct genc_linear_probing_hash_table_desc
{
	genc_key_hash_fn hash_fn;
	genc_hash_get_item_key_fn get_key_fn;
	genc_hash_key_equality_fn key_equality_fn;
	genc_item_is_empty_fn item_empty_fn;
	genc_item_clear_fn item_clear_fn;
	genc_realloc_fn realloc_fn;
	size_t bucket_size; /* bytes per item */
	uint8_t load_percent_grow_threshold;
	uint8_t load_percent_shrink_threshold;
};
typedef struct genc_linear_probing_hash_table_desc genc_linear_probing_hash_table_desc_t;


struct genc_linear_probing_hash_table
{
	genc_linear_probing_hash_table_light_t table;
	void* opaque;
	genc_linear_probing_hash_table_desc_t desc;
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



void genc_linear_probing_hash_table_desc_init(
	genc_linear_probing_hash_table_desc_t* desc,
	genc_key_hash_fn hash_fn,
	genc_hash_get_item_key_fn get_key_fn,
	genc_hash_key_equality_fn key_equality_fn,
	genc_item_is_empty_fn item_empty_fn,
	genc_item_clear_fn item_clear_fn,
	genc_realloc_fn realloc_fn,
	size_t bucket_size, /* bytes per item */
	uint8_t load_percent_grow_threshold,
	uint8_t load_percent_shrink_threshold);

genc_bool_t genc_linear_probing_hash_table_light_init(
	genc_linear_probing_hash_table_light_t* table,
	const genc_linear_probing_hash_table_desc_t* desc,
	void* opaque,
	size_t initial_capacity_pow2);

/* Zeroes out all the fields in the table struct - this leaves the table in a
 * known but unusable state. (Like init followed by destroy) */
void genc_lphtl_zero(genc_linear_probing_hash_table_light_t* table);

/* Returns the current number of items in the hash table. */
size_t genc_lphtl_count(genc_linear_probing_hash_table_light_t* table);

/* Returns the number of buckets allocated. */
size_t genc_lphtl_capacity(genc_linear_probing_hash_table_light_t* table);

/* Drops all items from the table and deallocates bucket array memory. */
void genc_lphtl_destroy(genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque);
/* Drops all items from the table but does not resize or deallocate it. */
void genc_lphtl_clear(genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque);

/* Inserts the given item into the hash table.
 * Returns NULL to report failure due to a duplicate or growth failure, pointer
 * to inserted bucket on success. */
void* genc_lphtl_insert_item(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* item);

bool genc_lphtl_grow_by(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	unsigned log2_grow_factor);
bool genc_lphtl_shrink_by(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	unsigned log2_shrink_factor);

genc_hash_t genc_lphtl_get_bucket_for_key(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* key);
void* genc_lphtl_find(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* key);
genc_lpht_insertion_test_result_t genc_lphtl_can_insert_item(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* item);
void genc_lphtl_remove(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* item);
void* genc_lphtl_first_item(genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque);
void* genc_lphtl_next_item(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* const opaque,
	void* cur_item);

/** Resizes the table, if necessary, so that it will not need resizing to hold
 * target_count items.
 * So if it currently has count items, where count < target_count, and we make
 * (target_count-count) successful insertions, none of those insertions will
 * trigger a resize operation. */
bool genc_lphtl_reserve_space(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque, size_t target_count);

#define genc_lphtl_first_obj(table, desc, opaque, type) \
GENC_CXX_CAST(type*, genc_lphtl_first_item(table, desc, opaque))

#define genc_lphtl_next_obj(table, desc, opaque, cur_obj, type) \
GENC_CXX_CAST(type*, genc_lphtl_next_item(table, desc, opaque, GENC_CXX_CAST(type*, cur_obj)))

#define genc_lphtl_find_obj(table, desc, opaque, key, type) \
GENC_CXX_CAST(type*, genc_lphtl_find(table, desc, opaque, key))

#define genc_lphtl_insert_obj(table, desc, opaque, new_obj, type) \
GENC_CXX_CAST(type*, genc_lphtl_insert_item(table, desc, opaque, GENC_CXX_CAST(type*, new_obj)))

#if defined(KERNEL) && defined(APPLE)
#undef ptrdiff_t
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
