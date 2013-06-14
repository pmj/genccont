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
	return k->i; // deliberately weak "algorithm" so we can easily test collisions
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
	bool verified = genc_lpht_verify(&hashtable);
	STAssertTrue(verified, @"Hash table should always pass verification after all the operations we run on it.");
	genc_lpht_destroy(&hashtable);
  
	[super tearDown];
}

- (void)testInsert
{
	test_value test = { 100, { 5 }, 200 };
	test_value test2 = { .key = { 6 }, .a = 101, .b = 201 };
	test_value* inserted = genc_lpht_insert_item(&hashtable, &test);
	test_value* inserted2 = genc_lpht_insert_item(&hashtable, &test2);

	STAssertTrue(inserted != NULL, @"Inserting should succeed");
  STAssertEquals(test.key.i, inserted->key.i, @"Inserted key must not change");
	
	STAssertTrue(inserted2 != NULL, @"Inserting should succeed");
  STAssertEquals(test2.key.i, inserted2->key.i, @"Inserted key must not change");
}

- (void)testInsertDuplication
{
	test_value test = { .key = { 6 }, .a = 101, .b = 201 };
	test_value test2 = { .key = { 6 }, .a = 101, .b = 201 };
	test_value* inserted = genc_lpht_insert_item(&hashtable, &test);
	test_value* inserted2 = genc_lpht_insert_item(&hashtable, &test2);

	STAssertTrue(inserted != NULL, @"Inserting should succeed");
  STAssertEquals(test.key.i, inserted->key.i, @"Inserted key must not change");
	
	STAssertTrue(inserted2 == NULL, @"Inserting should fail as it is a duplication");
}

- (void)testInsertCollision
{
	test_value test = { .key = { 6 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 14 }, .a = 101, .b = 201 };
	test_value* inserted = genc_lpht_insert_item(&hashtable, &test);
	test_value* inserted2 = genc_lpht_insert_item(&hashtable, &test2);

	STAssertTrue(inserted != NULL, @"Inserting should succeed");
  STAssertEquals(test.key.i, inserted->key.i, @"Inserted key must not change");
	
	STAssertTrue(inserted2 != NULL, @"Inserting should succeed");
  STAssertEquals(test2.key.i, inserted2->key.i, @"Inserted key must not change");
}

- (void)testFind
{
	test_value test = { .key = { 6 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 14 }, .a = 101, .b = 201 };
	genc_lpht_insert_item(&hashtable, &test);
	genc_lpht_insert_item(&hashtable, &test2);
	test_key key = {14};
	test_value* find = genc_lpht_find(&hashtable, &key);

	
	STAssertTrue(find != NULL, @"Find should succeed");
  STAssertEquals(test2.key.i, find->key.i, @"Find key must not change");
	
}

- (void)testCollision
{
	test_value test = { .key = { 6 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 14 }, .a = 101, .b = 201 };
	genc_lpht_insert_item(&hashtable, &test);
	genc_lpht_insert_item(&hashtable, &test2);
	test_key key = {14};
	
	test_value* bucket0 = hashtable.buckets;
	test_value* bucket = genc_lpht_find(&hashtable, &key);
	size_t bucket_num = bucket - bucket0;
	
	STAssertTrue(bucket != NULL, @"Should be able to find inserted item");
  STAssertEquals(bucket_num, 7lu, @"Hash modulo capacity yields 6, linear probing means it's stored in 7");
	STAssertEquals(bucket->a, 101u, @"Inserted value should match");
	STAssertEquals(bucket->b, 201u, @"Inserted value should match");
	
	key.i = 6;
	bucket = genc_lpht_find(&hashtable, &key);
	bucket_num = bucket - bucket0;
	
	STAssertTrue(bucket != NULL, @"Should be able to find inserted item");
  STAssertEquals(bucket_num, 6lu, @"This was inserted first, so it ends up at the immediate location");
	STAssertEquals(bucket->a, 107u, @"Inserted value should still match");
	STAssertEquals(bucket->b, 214u, @"Inserted value should match");

}

- (void)testWrapAroundCollision
{
	test_value test = { .key = { 7 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 15 }, .a = 101, .b = 201 };
	genc_lpht_insert_item(&hashtable, &test);
	genc_lpht_insert_item(&hashtable, &test2);
	test_key key = {15};
	
	test_value* bucket0 = hashtable.buckets;
	test_value* bucket = genc_lpht_find(&hashtable, &key);
	size_t bucket_num = bucket - bucket0;
	
	STAssertTrue(bucket != NULL, @"Should be able to find inserted item");
  STAssertEquals(bucket_num, 0lu, @"Hash modulo capacity yields 7, linear probing means it's stored in 0");
	STAssertEquals(bucket->a, 101u, @"Inserted value should match");
	STAssertEquals(bucket->b, 201u, @"Inserted value should match");
	
	key.i = 7;
	bucket = genc_lpht_find(&hashtable, &key);
	bucket_num = bucket - bucket0;
	
	STAssertTrue(bucket != NULL, @"Should be able to find inserted item");
  STAssertEquals(bucket_num, 7lu, @"Hash modulo capacity yields 7");
	STAssertEquals(bucket->a, 107u, @"Inserted value should match");
	STAssertEquals(bucket->b, 214u, @"Inserted value should match");

}
- (void)testWrapAroundandCollision
{
	/*Collision with key 7 so wrap around and fill bucket 0, fill bucket 1 and so then key 0 must go in bucket 2*/
	test_value test = { .key = { 7 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 15 }, .a = 101, .b = 201 };
	test_value test3 = { .key = { 1 }, .a = 177, .b = 299 };
	test_value test4 = { .key = { 0 }, .a = 111, .b = 231 };
	genc_lpht_insert_item(&hashtable, &test);
	genc_lpht_insert_item(&hashtable, &test2);
	genc_lpht_insert_item(&hashtable, &test3);
	genc_lpht_insert_item(&hashtable, &test4);
	test_key key = {15};
	
	test_value* bucket0 = hashtable.buckets;
	test_value* bucket = genc_lpht_find(&hashtable, &key);
	size_t bucket_num = bucket - bucket0;
	
	STAssertTrue(bucket != NULL, @"Should be able to find inserted item");
  STAssertEquals(bucket_num, 0lu, @"Must be able to find the bucket from a key");
	STAssertEquals(bucket->a, 101u, @"Inserted value should match");
	STAssertEquals(bucket->b, 201u, @"Inserted value should match");
	
	key.i = 7;
	bucket = genc_lpht_find(&hashtable, &key);
	bucket_num = bucket - bucket0;
	
	STAssertTrue(bucket != NULL, @"Should be able to find inserted item");
  STAssertEquals(bucket_num, 7lu, @"Must be able to find the bucket from a key");
	STAssertEquals(bucket->a, 107u, @"Inserted value should match");
	STAssertEquals(bucket->b, 214u, @"Inserted value should match");

	key.i = 1;
	bucket = genc_lpht_find(&hashtable, &key);
	bucket_num = bucket - bucket0;
	
	STAssertTrue(bucket != NULL, @"Should be able to find inserted item");
  STAssertEquals(bucket_num, 1lu, @"Must be able to find the bucket from a key");
	STAssertEquals(bucket->a, 177u, @"Inserted value should match");
	STAssertEquals(bucket->b, 299u, @"Inserted value should match");
	
	key.i = 0;
	bucket = genc_lpht_find(&hashtable, &key);
	bucket_num = bucket - bucket0;
	
	STAssertTrue(bucket != NULL, @"Should be able to find inserted item");
  STAssertEquals(bucket_num, 2lu, @"Must be able to find the bucket from a key");
	STAssertEquals(bucket->a, 111u, @"Inserted value should match");
	STAssertEquals(bucket->b, 231u, @"Inserted value should match");
}

- (void)testRemove
{
	test_value test = { .key = { 6 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 14 }, .a = 101, .b = 201 };
	test_value test3 = { .key = { 22 }, .a = 104, .b = 209 };

	test_value* inserted = genc_lpht_insert_item(&hashtable, &test);
	genc_lpht_insert_item(&hashtable, &test2);
	genc_lpht_insert_item(&hashtable, &test3);

	genc_lpht_remove(&hashtable, inserted);

	test_key key = {6};
	test_value* find = genc_lpht_find(&hashtable, &key);
	
	STAssertTrue(find == NULL, @"Find should fail as key is deleted");
  
	test_key key2 = {14};
	find = genc_lpht_find(&hashtable, &key2);
	
	STAssertTrue(find != NULL, @"Find should succeed");
	STAssertEquals(test2.key.i, find->key.i, @"Find key must not change");
	
	test_key key3 = {22};
	find = genc_lpht_find(&hashtable, &key3);
	
	STAssertTrue(find != NULL, @"Find should succeed");
	STAssertEquals(test3.key.i, find->key.i, @"Find key must not change");
	
}

- (void)testRemoveandInsert
{
  /*Insert 3 values with the same key modulo remove the first value, the other two should shift buckets add a new third value with the same key modulo*/
	test_value test = { .key = { 6 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 14 }, .a = 101, .b = 201 };
	test_value test3 = { .key = { 22 }, .a = 104, .b = 209 };
	test_value test4 = { .key = { 30 }, .a = 154, .b = 279 };

	test_value* inserted = genc_lpht_insert_item(&hashtable, &test);
	genc_lpht_insert_item(&hashtable, &test2);
	genc_lpht_insert_item(&hashtable, &test3);

	genc_lpht_remove(&hashtable, inserted);
	genc_lpht_insert_item(&hashtable, &test4);

	test_key key = {30};
	test_value* find = genc_lpht_find(&hashtable, &key);
	
	STAssertTrue(find != NULL, @"Find should succeed");
	STAssertEquals(test4.key.i, find->key.i, @"Find key must not change");
		
}

- (void)testAutomaticGrow
{

	test_value test = { .key = { 6 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 14 }, .a = 101, .b = 201 };
	test_value test3 = { .key = { 22 }, .a = 104, .b = 209 };
	test_value test4 = { .key = { 30 }, .a = 154, .b = 279 };
	test_value test5 = { .key = { 0 }, .a = 143, .b = 233 };
	test_value test6 = { .key = { 5 }, .a = 185, .b = 294 };

	genc_lpht_insert_item(&hashtable, &test);
	genc_lpht_insert_item(&hashtable, &test2);
	genc_lpht_insert_item(&hashtable, &test3);
	genc_lpht_insert_item(&hashtable, &test4);
	genc_lpht_insert_item(&hashtable, &test5);
	genc_lpht_insert_item(&hashtable, &test6);
	
	size_t sizeOfHashTable = genc_lpht_capacity(&hashtable);


	test_key key = {22};
	test_value* find = genc_lpht_find(&hashtable, &key);
	
	STAssertTrue(8 != sizeOfHashTable, @"The hashtable should have automatically grown");
	STAssertEquals(test3.key.i, find->key.i, @"Find key must not change");

}

- (void)testGrowing
{

	test_value test = { .key = { 6 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 14 }, .a = 101, .b = 201 };
	test_value test3 = { .key = { 22 }, .a = 104, .b = 209 };

	genc_lpht_insert_item(&hashtable, &test);
	genc_lpht_insert_item(&hashtable, &test2);
	genc_lpht_insert_item(&hashtable, &test3);
	
	size_t sizeOfHashTable = genc_lpht_capacity(&hashtable);

	STAssertTrue(8 == sizeOfHashTable, @"The hashtable should not have automatically grown");
	
	genc_lpht_grow_by(&hashtable, 1);
	
	size_t sizeOfNewHashTable = genc_lpht_capacity(&hashtable);
	STAssertTrue(16 == sizeOfNewHashTable, @"The hashtable should have grown");

}

- (void)testFindAfterGrowing
{

	test_value test = { .key = { 6 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 14 }, .a = 101, .b = 201 };
	test_value test3 = { .key = { 22 }, .a = 104, .b = 209 };

	genc_lpht_insert_item(&hashtable, &test);
	genc_lpht_insert_item(&hashtable, &test2);
	genc_lpht_insert_item(&hashtable, &test3);
	
	genc_lpht_grow_by(&hashtable, 1);
	
	test_value test4 = { .key = { 30 }, .a = 111, .b = 211 };
	genc_lpht_insert_item(&hashtable, &test4);

	test_key key = {14};
	test_key key2 = {30};

	test_value* find = genc_lpht_find(&hashtable, &key);
	STAssertEquals(test2.key.i, find->key.i, @"Find key must not change");

	find = genc_lpht_find(&hashtable, &key2);
	STAssertEquals(test4.key.i, find->key.i, @"Find key must not change");

}

- (void)testShrinking
{
	//first grow the hashtable to then shrink it down
	genc_lpht_grow_by(&hashtable, 1);
	size_t sizeOfHashTable = genc_lpht_capacity(&hashtable);
	STAssertTrue(16 == sizeOfHashTable, @"The hashtable should now have grown");
	
	test_value test = { .key = { 6 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 9 }, .a = 101, .b = 201 };
	test_value test3 = { .key = { 13 }, .a = 104, .b = 209 };

	genc_lpht_insert_item(&hashtable, &test);
	genc_lpht_insert_item(&hashtable, &test2);
	genc_lpht_insert_item(&hashtable, &test3);
	
	test_key key = {6};
	test_key key2 = {9};
	test_key key3 = {13};

	test_value* find = genc_lpht_find(&hashtable, &key);
	STAssertEquals(test.key.i, find->key.i, @"Find key must not change");
	
	find = genc_lpht_find(&hashtable, &key2);
	STAssertEquals(test2.key.i, find->key.i, @"Find key must not change");

	find = genc_lpht_find(&hashtable, &key3);
	STAssertEquals(test3.key.i, find->key.i, @"Find key must not change");
	
	//Now shrink the table
	genc_lpht_shrink_by(&hashtable, 1);
	size_t sizeShrunkOfHashTable = genc_lpht_capacity(&hashtable);
	STAssertTrue(8 == sizeShrunkOfHashTable, @"The hashtable should now have shrunk");

	find = genc_lpht_find(&hashtable, &key2);
	STAssertEquals(test2.key.i, find->key.i, @"Find key must not change");

	find = genc_lpht_find(&hashtable, &key3);
	STAssertEquals(test3.key.i, find->key.i, @"Find key must not change");
}

- (void)testIterating
{
	test_value test = { .key = { 0 }, .a = 107, .b = 214 };
	test_value test2 = { .key = { 1 }, .a = 101, .b = 201 };
	test_value test3 = { .key = { 2 }, .a = 104, .b = 229 };
	test_value test4 = { .key = { 3 }, .a = 114, .b = 239 };
	test_value test5 = { .key = { 4 }, .a = 124, .b = 249 };

	genc_lpht_insert_item(&hashtable, &test);
	genc_lpht_insert_item(&hashtable, &test2);
	genc_lpht_insert_item(&hashtable, &test3);
	genc_lpht_insert_item(&hashtable, &test4);
	genc_lpht_insert_item(&hashtable, &test5);
	
	BOOL array[5] = {FALSE, FALSE, FALSE, FALSE, FALSE};
	test_value* item = genc_lpht_first_item(&hashtable);
	int count = 0;
	
	do
	{
		uint32_t key = item->key.i;
		array[key] = TRUE;
		count++;
	}
	while ((item = genc_lpht_next_item(&hashtable, item)));
	
	STAssertEquals(5, count, @"5 items were looped through");
	STAssertTrue(array[0], @"All the items in the array were seen");
	STAssertTrue(array[1], @"All the items in the array were seen");
	STAssertTrue(array[2], @"All the items in the array were seen");
	STAssertTrue(array[3], @"All the items in the array were seen");
	STAssertTrue(array[4], @"All the items in the array were seen");


}


@end
