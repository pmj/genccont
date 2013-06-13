//
//  LinearProbingHashTableTest.m
//  LinearProbingHashTableTest
//
//  Created by Laura Dennis on 6/11/13.
//
//

#import "LinearProbingHashTableTest.h"

// Values in the hash table will be keyed using this type:
struct test_key
{
	uint32_t i;
};
typedef struct test_key test_key;

// The type of values to store in our hash table
struct test_value
{
	uint32_t a;
	test_key key;
	uint32_t b;
};
typedef struct test_value test_value;


static genc_hash_t hash_fn(void* key, void* opaque)
{
	test_key* k = key;
	return genc_hash_uint32(k->i);
}

static void* get_item_key(void* item, void* opaque)
{
	test_value* v = item;
	return &v->key;
}

static genc_bool_t key_equality_fn(void* key1, void* key2, void* opaque)
{
	test_key* k1 = key1;
	test_key* k2 = key2;
	return k1->i == k2->i;
}

static genc_bool_t is_empty_fn(void* item, void* opaque)
{
	test_value* v = item;
	return v->key.i == UINT32_MAX && v->a == UINT32_MAX && v->b == UINT32_MAX;
}

static void clear_fn(void* item, void* opaque)
{
	test_value* v = item;
	v->a = UINT32_MAX;
	v->key.i = UINT32_MAX;
	v->b = UINT32_MAX;
}


static void* realloc_fn(void* old_ptr, size_t old_size, size_t new_size, void* opaque)
{
	return realloc(old_ptr, new_size);
}

@implementation LinearProbingHashTableTest

- (void)setUp
{
	[super setUp];
    
	bool ok = genc_linear_probing_hash_table_init(
		&hashtable,
		hash_fn, get_item_key, key_equality_fn, is_empty_fn, clear_fn, realloc_fn,
		NULL /* opaque ptr */,
		sizeof(test_value),
		8);
	STAssertTrue(ok, @"Initialising the hash table should always succeed.");
}

- (void)tearDown
{
	genc_lpht_destroy(&hashtable);
  
	[super tearDown];
}

- (void)testInsert
{
  
}

@end
