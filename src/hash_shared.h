/*
Copyright (c) 2011-2013 Phil Jordan <phil@philjordan.eu>

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
 * This library implements multiple variants of hash tables. This shared header
 * collects type and function declarations which are useful for more than one of
 * them.
 */

#ifndef GENCCONT_HASH_SHARED_H
#define GENCCONT_HASH_SHARED_H

#include "util.h"

#include <stdint.h>


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
typedef genc_hash_t(*genc_key_hash_fn)(void* key, void* opaque);

/* key accessor function - extracts pointer to the key from the item whose
 * chaining head is passed in. Chaining table uses a different one. */
typedef void*(*genc_hash_get_item_key_fn)(void* item, void* opaque);

/* key-key equality function to be implemented by the client
 * Shall return false(0) for inequality and true(1) for equality. */
typedef genc_bool_t(*genc_hash_key_equality_fn)(void* key1, void* key2, void* opaque);

/* Open-addressed hash tables need to know if an entry is empty or occupied,
 * and as the buckets are entirely client-defined, the "empty" value is also
 * client-defined. */
typedef genc_bool_t(*genc_item_is_empty_fn)(void* item, void* opaque);

/* Marks the open-addressed bucket as empty. */
typedef void(*genc_item_clear_fn)(void* item, void* opaque);



/* memory re-allocation function. Semantics similar to C's realloc(), copying allowed.
 * new_size = 0 frees memory
 * old_size = 0 allocates a new block.
 * Additionally, if the table should have a fixed size, disable shrinking by
 * setting the shrink threshold to 0 and prevent growing by returning NULL from
 * the realloc function when old_size > 0 && new_size > 0. You're still expected
 * to provide the initial memory.
 * Opaque is just passed through from the hash table and can be used for
 * selecting a memory pool, etc.
 */
typedef void*(*genc_realloc_fn)(void* old_ptr, size_t old_size, size_t new_size, void* opaque);


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
genc_bool_t genc_uint32_keys_equal(void* id1, void* id2, void* opaque_unused);
size_t genc_uint64_key_hash(void* item, void* opaque_unused);
genc_bool_t genc_uint64_keys_equal(void* id1, void* id2, void* opaque_unused);

/// Useful in cases where the key pointer value is the key itself (i.e. not to be dereferenced)
size_t genc_pointer_key_hash(void* key, void* opaque_unused);
genc_bool_t genc_pointer_keys_equal(void* key1, void* key2, void* opaque_unused);

size_t genc_hash_combine(size_t seed, size_t hash_value);

/* Helpers for sizing power-of-2-capacity hash tables: */

static GENC_INLINE int genc_is_pow2(size_t val)
{
	return val != 0ul && 0ul == (val & (val - 1ul));
}

static GENC_INLINE int genc_log2_size(size_t val)
{
	return val == 0 ? -1 : (int)(sizeof(size_t) * 8 - 1 - __builtin_clzl(val));
}

static GENC_INLINE int genc_log2_size_roundup(size_t val)
{
	int log2 = genc_log2_size(val);
	if (genc_is_pow2(val)) return log2;
	return log2 + 1;
}

/// Helper macro for generating key getter functions (genc_hash_get_item_key_fn) for simple structs
#define GENC_CHT_STRUCT_KEY_GETTER(STRUCTNAME, CHT_HEAD_MEMBER, KEY_MEMBER) \
static void* STRUCTNAME ## _get_key(struct slist_head* item, void* opaque) \
{ \
	return &genc_container_of_notnull(item, struct STRUCTNAME, CHT_HEAD_MEMBER)->KEY_MEMBER; \
}

/// Helper macro for generating key getter functions (genc_hash_get_item_key_fn) for pointer fields in structs
/** The generated getter returns the field's pointer value itself, not the pointer to the field. */
#define GENC_CHT_STRUCT_POINTER_KEY_GETTER(STRUCTNAME, CHT_HEAD_MEMBER, KEY_MEMBER) \
static void* STRUCTNAME ## _get_key(struct slist_head* item, void* opaque) \
{ \
	return genc_container_of_notnull(item, struct STRUCTNAME, CHT_HEAD_MEMBER)->KEY_MEMBER; \
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
