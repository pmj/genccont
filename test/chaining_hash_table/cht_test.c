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

#include "../../src/chaining_hash_table.h"
#include <stdlib.h>
#include <assert.h>

static void* cht_test_realloc(void* old, size_t old_size, size_t new_size)
{
	return realloc(old, new_size);
}

typedef struct genc_chaining_hash_table genc_chaining_hash_table_t;
typedef struct slist_head slist_head_t;

struct test_entry
{
	struct slist_head hash_head;
	unsigned key;
	unsigned val;
};
typedef struct test_entry test_entry_t;

static genc_hash_t cht_test_hash(void* key, void* opaque)
{
	assert(!opaque);
	return genc_hash_uint32(*(unsigned*)key);
}

static void* cht_test_get_key(struct slist_head* hash_head, void* opaque)
{
	struct test_entry* e = genc_container_of(hash_head, struct test_entry, hash_head);
	return &e->key;
}

static int cht_test_keys_equal(void* key1, void* key2, void* opaque)
{
	assert(!opaque);
	assert(key1);
	assert(key2);
	return (*(unsigned*)key1) == (*(unsigned*)key2);
}

static test_entry_t entry1 = { {NULL}, 1, 100 };
static test_entry_t entry1dup = { {NULL}, 1, 1000 };
static test_entry_t entry2 = { {NULL}, 2, 200 };
static test_entry_t entry3 = { {NULL}, 3, 300 };
static test_entry_t entry4 = { {NULL}, 4, 400 };
static test_entry_t entry5 = { {NULL}, 5, 500 };

int main()
{
	genc_chaining_hash_table_t table;
	unsigned key = 0;
	int res = 0;
	slist_head_t** ref = NULL;
	size_t count = 0, capacity = 0;
	slist_head_t* removed = NULL;
	
	genc_chaining_hash_table_init(&table, cht_test_hash, cht_test_get_key, cht_test_keys_equal, cht_test_realloc, NULL, 4);
	
	/* Test lookup in an empty table */
	key = 1;
	ref = genc_cht_find_ref(&table, &key);
	assert(ref);
	assert(!*ref);
	
	/* Test simple insertion */
	res = genc_cht_insert_item(&table, &entry1.hash_head);
	assert(res);
	
	/* Test failure to insert duplicate item */
	res = genc_cht_insert_item(&table, &entry1dup.hash_head);
	assert(!res);
	
	/* Test successful lookup */
	ref = genc_cht_find_ref(&table, &key);
	assert(ref && *ref);
	assert(*ref == &entry1.hash_head);
	
	/* insert more items */
	res = genc_cht_insert_item(&table, &entry2.hash_head);
	assert(res);
	res = genc_cht_insert_item(&table, &entry3.hash_head);
	assert(res);

	/* 3 items now in table */
	count = genc_cht_count(&table);
	assert(count == 3);
	
	/* initial size is 4, default grow load factor threshold is 0.7, so we should now be at capacity 8 */
	capacity = genc_cht_capacity(&table);
	assert(capacity == 8);
	
	/* yet more items */
	res = genc_cht_insert_item(&table, &entry4.hash_head);
	assert(res);
	res = genc_cht_insert_item(&table, &entry5.hash_head);
	assert(res);
	
	/* Test successful lookup */
	key = 3;
	ref = genc_cht_find_ref(&table, &key);
	assert(ref && *ref);
	assert(*ref == &entry3.hash_head);

	/* Test failed lookup */
	key = 6;
	ref = genc_cht_find_ref(&table, &key);
	assert(ref && !*ref);

	/* Test removal */
	key = 3;
	removed = genc_cht_remove(&table, &key);
	assert(removed == &entry3.hash_head);

	/* Test that removed element is no longer present */
	key = 3;
	ref = genc_cht_find_ref(&table, &key);
	assert(ref && !*ref);
	
	/* Test failure to remove element which does not exist */
	key = 3;
	removed = genc_cht_remove(&table, &key);
	assert(!removed);
	
	/* test explicit shrinking by factor 4 */
	genc_cht_shrink_by(&table, 2);
	capacity = genc_cht_capacity(&table);
	assert(capacity == 2);
	
	/* Test lookups still work after shrinking (there will be collisions) */
	key = 1;
	ref = genc_cht_find_ref(&table, &key);
	assert(ref && *ref);
	assert(*ref == &entry1.hash_head);
	key = 2;
	ref = genc_cht_find_ref(&table, &key);
	assert(ref && *ref);
	assert(*ref == &entry2.hash_head);
	key = 4;
	ref = genc_cht_find_ref(&table, &key);
	assert(ref && *ref);
	assert(*ref == &entry4.hash_head);
	key = 5;
	ref = genc_cht_find_ref(&table, &key);
	assert(ref && *ref);
	assert(*ref == &entry5.hash_head);
	
	/* re-insert the removed element; this should have grown the table again */
	res = genc_cht_insert_item(&table, &entry3.hash_head);
	assert(res);
	capacity = genc_cht_capacity(&table);
	assert(capacity == 8);

	/* explicitly grow the table to be huge */
	genc_cht_grow_by(&table, 8);
	capacity = genc_cht_capacity(&table);
	assert(capacity == 8 * 256);
	
	genc_cht_destroy(&table);
	return 0;
}
