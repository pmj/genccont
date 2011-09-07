/*
Copyright (c) 2011 Phil Jordan <phil@philjordan.eu>

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
 * A simple, generic hash table with the following characteristics and expectations:
 * - memory of table entries is managed by the library's client, entries are never copied
 * - entries are expected to be structs which embed an slist_head member for chaining
 * - Hash function and key equality function are supplied as function pointers
 * 
 */


#ifndef GENCCONT_CHAINING_HASH_TABLE_H
#define GENCCONT_CHAINING_HASH_TABLE_H

/* Chaining is done with singly linked list */
#include "slist.h"

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

/* Hash values are expected to span the entire address space or the key space,
 * whichever is smaller.
 * (i.e. it's okay to produce 32-bit hash values on a 64-bit
 * system if there are only 1<<32 unique keys) */
typedef size_t genc_hash_t;

/* Client functions supplied to the library may accept an additional opaque
 * pointer for parameterisation. None of the functions may return different
 * results for the same inputs at different times. */

/* key hashing function to be implemented by the client; will be passed pointer
 * to the key extracted from an item or passed to the library directly */
typedef genc_hash_t(*genc_chaining_key_hash_fn)(void* item, void* opaque);

/* key accessor function - extracts pointer to the key from the item whose
 * chaining head is passed in */
typedef void*(*genc_chaining_hash_get_item_key_fn)(struct slist_head* item, void* opaque);

/* key-key equality function to be implemented by the client
 * Shall return 0 for inequality and 1 for equality. */
typedef int(*genc_chaining_hash_key_equality_fn)(void* key1, void* key2, void* opaque);

/* memory re-allocation function. Semantics similar to C's realloc(), copying allowed.
 * new_size = 0 frees memory
 * old_size = 0 allocates a new block.
 * Additionally, if the table should have a fixed size, disable shrinking by
 * setting the shrink threshold to 0 and prevent growing by returning NULL from
 * the realloc function when old_size > 0 && new_size > 0. You're still expected
 * to provide the initial memory. */
typedef void*(*genc_realloc_fn)(void* old_ptr, size_t old_size, size_t new_size);

struct genc_chaining_hash_table;

/* Initialises the empty hash table with the given function implementations and capacity.
 * Defaults (70, 0) are used for load factor percentage thresholds for growing and shrinking. */
int genc_chaining_hash_table_init(
	struct genc_chaining_hash_table* table,
	genc_chaining_key_hash_fn hash_fn,
	genc_chaining_hash_get_item_key_fn get_key_fn,
	genc_chaining_hash_key_equality_fn key_equality_fn,
	genc_realloc_fn realloc_fn,
	void* opaque,
	size_t initial_capacity_pow2);

/* initialises the hash table with non-default thresholds.
 * Table will only grow on insertion and shrink on removal.
 * Grow threshold should be somewhat more than 2x shrink threshold to avoid
 * oscillation.
 * Capacity is always a power of 2.
 */
int genc_chaining_hash_table_init_ext(
	struct genc_chaining_hash_table* table,
	genc_chaining_key_hash_fn hash_fn,
	genc_chaining_hash_get_item_key_fn get_key_fn,
	genc_chaining_hash_key_equality_fn key_equality_fn,
	genc_realloc_fn realloc_fn,
	void* opaque,
	size_t initial_capacity_pow2,
	uint8_t load_percent_grow_threshold,
	uint8_t load_percent_shrink_threshold);

/* Returns the current number of items in the hash table. */
size_t genc_cht_count(struct genc_chaining_hash_table* table);

size_t genc_cht_capacity(struct genc_chaining_hash_table* table);

/* Drops all items from the table (without deleting them) and deallocates bucket array memory. */
void genc_cht_destroy(struct genc_chaining_hash_table* table);

/* Inserts the given item into the hash table.
 * Returns 0 to report failure due to a duplicate, 1 on success. */
int genc_cht_insert_item(struct genc_chaining_hash_table* table, struct slist_head* item);

/* Looks up the key in the table, returning the matching item if present, or NULL otherwise. */
struct slist_head* genc_cht_find(struct genc_chaining_hash_table* table, void* key);

/* Looks up the key in the table, returning the reference pointing to the
 * matching item, or something pointing to NULL if not found. The reference may be passed to
 * genc_cht_remove_ref() for efficient removal. */
struct slist_head** genc_cht_find_ref(struct genc_chaining_hash_table* table, void* key);

/* Removes the item referred to by item_ref from the hash table, returning it.
 * Deallocation, like allocation, is the responsibility of the caller.
 */
struct slist_head* genc_cht_remove_ref(struct genc_chaining_hash_table* table, struct slist_head** item_ref);

/* Calls genc_cht_find_ref() and calls genc_cht_remove_ref() with the result if a match was found. */
struct slist_head* genc_cht_remove(struct genc_chaining_hash_table* table, void* key);

/* Shrink the capacity of the table by a factor of 1 << log2_shrink_factor */
void genc_cht_shrink_by(struct genc_chaining_hash_table* table, unsigned log2_shrink_factor);
/* Grow the capacity of the table by a factor of 1 << log2_grow_factor */
void genc_cht_grow_by(struct genc_chaining_hash_table* table, unsigned log2_grow_factor);

struct genc_chaining_hash_table
{
	genc_chaining_key_hash_fn hash_fn;
	genc_chaining_hash_get_item_key_fn get_key_fn;
	genc_chaining_hash_key_equality_fn key_equality_fn;
	genc_realloc_fn realloc_fn;
	void* opaque;
	size_t capacity;
	size_t item_count;
	struct slist_head** buckets;
	uint8_t load_percent_grow_threshold;
	uint8_t load_percent_shrink_threshold;
};




/* Useful integer hash functions, not used directly but can be used to implement
 * client hash functions */
size_t genc_hash_uint32(uint32_t k);
size_t genc_hash_uint64(uint64_t k);
static GENC_INLINE size_t genc_hash_size(size_t k)
{
#if defined(__LP64__) || defined(_WIN64)
	return genc_hash_uint64(k);
#else
	return genc_hash_uint32(k);
#endif
}

size_t genc_uint32_key_hash(void* item, void* opaque_unused);
size_t genc_uint64_key_hash(void* item, void* opaque_unused);
int genc_uint64_keys_equal(void* id1, void* id2, void* opaque_unused);

#ifdef __cplusplus
} /* extern "C" */
#endif

#define genc_cht_find_obj(table, key, type, header_name) \
genc_container_of(genc_cht_find((table), (key)), type, header_name)

#define genc_cht_for_each_ref(TABLE, ENTRY_VAR, CUR_HEAD_PTR_VAR, BUCKET_VAR, ENTRY_TYPE, TABLE_HEAD_MEMBER_NAME) \
for (BUCKET_VAR = 0, CUR_HEAD_PTR_VAR = ((TABLE)->buckets + BUCKET_VAR); \
	BUCKET_VAR < (TABLE)->capacity; \
	++BUCKET_VAR, CUR_HEAD_PTR_VAR = (TABLE)->buckets + BUCKET_VAR) \
		genc_slist_for_each_ref(ENTRY_VAR, CUR_HEAD_PTR_VAR, ENTRY_TYPE, TABLE_HEAD_MEMBER_NAME)


#if defined(KERNEL) && defined(APPLE)
#undef ptrdiff_t
#endif


#endif
