#include "linear_probing_hash_table.h"
#include <string.h>

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
	size_t initial_capacity_pow2)
{
	return genc_linear_probing_hash_table_init_ext(
		table, hash_fn, get_key_fn, key_equality_fn,
		item_empty_fn, item_clear_fn, realloc_fn,
		opaque, bucket_size, initial_capacity_pow2, 70, 0);
}

static void* alloc_empty_buckets(
	genc_realloc_fn realloc_fn, genc_item_clear_fn item_clear_fn,
	size_t capacity, size_t bucket_size, void* opaque)
{
	void* buckets = realloc_fn(NULL, 0, capacity * bucket_size, opaque);
	if (!buckets)
		return NULL;
	
	// set all buckets to empty
	char* bucket = GENC_CXX_CAST(char*, buckets);
	for (size_t i = 0; i < capacity; ++i)
	{
		item_clear_fn(bucket, opaque);
		bucket += bucket_size;
	}
	return buckets;
}

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
	uint8_t load_percent_shrink_threshold)
{
	void* buckets;
	if (!genc_is_pow2(initial_capacity_pow2))
		return 0;
	if (SIZE_MAX / bucket_size < initial_capacity_pow2)
		return 0; // overflow
	
	buckets = alloc_empty_buckets(
		realloc_fn, item_clear_fn, initial_capacity_pow2, bucket_size, opaque);
	
	table->hash_fn = hash_fn;
	table->get_key_fn = get_key_fn;
	table->key_equality_fn = key_equality_fn;
	table->item_empty_fn = item_empty_fn;
	table->item_clear_fn = item_clear_fn;
	table->realloc_fn = realloc_fn;
	table->opaque = opaque;
	table->bucket_size = bucket_size;
	table->capacity = initial_capacity_pow2;
	table->item_count = 0;
	table->buckets = buckets;
	table->load_percent_grow_threshold = load_percent_grow_threshold;
	table->load_percent_shrink_threshold = load_percent_shrink_threshold;
	
	return 1;
}

/* Returns the current number of items in the hash table. */
size_t genc_lpht_count(struct genc_linear_probing_hash_table* table)
{
	return table->item_count;
}

/* Returns the number of buckets allocated. */
size_t genc_lpht_capacity(struct genc_linear_probing_hash_table* table)
{
	return table->capacity;
}

/* Drops all items from the table and deallocates bucket array memory. */
void genc_lpht_destroy(struct genc_linear_probing_hash_table* table)
{
	if (table->buckets)
	{
		table->realloc_fn(table->buckets, table->capacity * table->bucket_size, 0, table->opaque);
		table->buckets = NULL;
		table->capacity = 0;
		table->item_count = 0;
	}
}

/* locates the bucket which either matches key or which we can insert an item
 * with that key into */
static void* genc_lpht_find_or_empty(
	struct genc_linear_probing_hash_table* table, void* key, bool* out_found)
{
	genc_hash_t start_idx = genc_lpht_get_bucket_for_key(table, key);
	
	*out_found = false;
	genc_hash_t idx = start_idx;
	do
	{
		char* bucket = GENC_CXX_CAST(char*, table->buckets);
		bucket += table->bucket_size * idx;
		
		// stop if bucket is empty or if we found an item which matches key
		if (table->item_empty_fn(bucket, table->opaque))
			return bucket;
		void* bucket_key = table->get_key_fn(bucket, table->opaque);
		if (table->key_equality_fn(bucket_key, key, table->opaque))
		{
			*out_found = true;
			return bucket;
		}
		
		
		++idx;
		if (idx >= table->capacity)
			idx = 0;
	} while (idx != start_idx);
	return NULL; // edge case: table is at full capacity and doesn't contain item with key
}

// pure insertion, without the bookkeeping
static void* genc_lpht_insert_item_into_table(
	struct genc_linear_probing_hash_table* table, void* item)
{
	void* item_key = table->get_key_fn(item, table->opaque);
	bool found = false;
	void* bucket = genc_lpht_find_or_empty(table, item_key, &found);
	
	if (!bucket || found)
		return NULL; // table is full, or item exists
	
	// insert the item
	memcpy(bucket, item, table->bucket_size);
	return bucket;
}

/* Inserts the given item into the hash table.
 * Returns NULL to report failure due to a duplicate or growth failure, pointer
 * to inserted bucket on success. */
void* genc_lpht_insert_item(
	struct genc_linear_probing_hash_table* table, void* item)
{
	unsigned new_load;
	if (!item) return 0;
	
	new_load = (unsigned)(100ul * (table->item_count + 1ul) / table->capacity);
	if (new_load > table->load_percent_grow_threshold || new_load >= 100)
	{
		int factor_log2 = genc_log2_size(new_load / table->load_percent_grow_threshold);
		if (new_load > ((unsigned)table->load_percent_grow_threshold) << factor_log2)
			++factor_log2;
		
		/*printf("New load factor %d%%, growth threshold reached. Growing by factor 1 << %d (%u).\n", new_load, factor_log2, 1u << factor_log2);*/
		genc_lpht_grow_by(table, factor_log2);
	}
	
	void* inserted = genc_lpht_insert_item_into_table(table, item);
	if (!inserted)
		return NULL;
	
	++table->item_count;
	return inserted;
}

/* Looks up the key in the table, returning the matching item if present, or NULL otherwise. */
void* genc_lpht_find(struct genc_linear_probing_hash_table* table, void* key)
{
	bool found = false;
	void* bucket = genc_lpht_find_or_empty(table, key, &found);
	return found ? bucket : NULL;
}

/* Hashes the key and returns the bucket into which the key falls */
genc_hash_t genc_lpht_get_bucket_for_key(
	struct genc_linear_probing_hash_table* table, void* key)
{
	genc_hash_t hash = table->hash_fn(key, table->opaque);
	hash &= (table->capacity - 1ul);
	return hash;
}

static GENC_INLINE genc_bool_t idx_between(
	genc_hash_t idx, genc_hash_t start_exc, genc_hash_t end_exc, genc_hash_t capacity)
{
	genc_hash_t mask = capacity - 1;
	// calculate distances idx->end_exc and start_exc->end_exc using modular arithmetic
	genc_hash_t idx_delta = (end_exc - idx) & mask;
	genc_hash_t start_delta = (end_exc - start_exc) & mask;
	
	return idx_delta < start_delta;
}

/* Removes the item from the hash table.
 * Deallocation, like allocation, is the responsibility of the caller.
 */
void genc_lpht_remove(struct genc_linear_probing_hash_table* table, void* item)
{
	if (item && !table->item_empty_fn(item, table->opaque))
	{
		table->item_clear_fn(item, table->opaque);
		--table->item_count;
		
		const size_t bucket_size = table->bucket_size;
		void* const opaque = table->opaque;
		genc_item_clear_fn item_clear_fn = table->item_clear_fn;
		genc_item_is_empty_fn item_empty_fn = table->item_empty_fn;
		
		// need to move up any displaced items which would now be unreachable
		char* buckets = GENC_CXX_CAST(char*, table->buckets);
		char* empty_bucket = GENC_CXX_CAST(char*, item);
		genc_hash_t start_idx = (empty_bucket - buckets) / table->bucket_size;
		genc_hash_t empty_idx = start_idx;
		genc_hash_t idx = (start_idx + 1) & (table->capacity - 1);
		/* all consecutive non-empty buckets are reachable, so keep going until we
		 * find an empty one. */
		char* bucket = buckets + idx * table->bucket_size;
		while (!item_empty_fn(bucket, table->opaque))
		{
			/* If current item is not reachable from its true slot, we need to move it
			 * into the empty one */
			void* item_key = table->get_key_fn(bucket, table->opaque);
			genc_hash_t key_bucket_idx = genc_lpht_get_bucket_for_key(table, item_key);
			if (idx_between(empty_idx, key_bucket_idx, idx, table->capacity))
			{
				char* moving_item = buckets + key_bucket_idx * bucket_size;
				memcpy(empty_bucket, moving_item, bucket_size);
				item_clear_fn(moving_item, opaque);
				empty_idx = idx;
				empty_bucket = bucket;
			}
			
			++idx;
			idx &= (table->capacity - 1);
			bucket = buckets + idx * table->bucket_size;
		}

		// shrink if necessary
		unsigned new_load = 0;
		--table->item_count;
		new_load = (unsigned)(100ull * (table->item_count) / table->capacity);

		if (new_load > 0 && new_load < table->load_percent_shrink_threshold)
		{
			int factor_log2 = genc_log2_size(table->load_percent_shrink_threshold / new_load);
			genc_lpht_shrink_by(table, factor_log2);
		}
	}
}

/* Shrink the capacity of the table by a factor of 1 << log2_shrink_factor */
void genc_lpht_shrink_by(struct genc_linear_probing_hash_table* table, unsigned log2_shrink_factor)
{
	const size_t old_capacity = table->capacity;
	// don't shrink it down so far that the contents no longer fits
	while (table->item_count > (old_capacity >> log2_shrink_factor))
	{
		if (log2_shrink_factor < 1)
			return;
		--log2_shrink_factor;
	}
	
	const size_t bucket_size = table->bucket_size;
	void* const opaque = table->opaque;
	
	// TODO: resize in-place
	const size_t new_capacity = old_capacity >> log2_shrink_factor;
	
	// alloc new bucket array
	const genc_realloc_fn realloc_fn = table->realloc_fn;
	char* new_buckets = GENC_CXX_CAST(
		char*, alloc_empty_buckets(realloc_fn, table->item_clear_fn, new_capacity, bucket_size, opaque));
	if (!new_buckets)
		return;
	
	// re-insert items into new buckets
	char* old_buckets = GENC_CXX_CAST(char*, table->buckets);
	table->buckets = new_buckets;
	table->capacity = new_capacity;
	
	char* old_bucket = old_buckets;
	for (genc_hash_t idx = 0; idx < old_capacity; ++idx, old_bucket += bucket_size)
	{
		if (!table->item_empty_fn(old_bucket, opaque)
		    && !genc_lpht_insert_item_into_table(table, old_bucket))
		{
			// failed to move item across, give up
			table->buckets = old_buckets;
			table->capacity = old_capacity;
			realloc_fn(
				new_buckets, new_capacity * bucket_size, 0, opaque);
			return;
		}
	}
	
	realloc_fn(
		old_buckets, old_capacity * bucket_size, 0, opaque);
	return;
}

/* Grow the capacity of the table by a factor of 1 << log2_grow_factor */
void genc_lpht_grow_by(
	struct genc_linear_probing_hash_table* table, unsigned log2_grow_factor)
{
	/* To resize in-place, we take a slightly sneaky approach:
	 * Try to re-insert every item that's already in the bucket (now larger) array.
	 * If insertion fails, that's because it's already in the right place. 
	 * If it succeeds, we can vacate its original location. */
	
	char* buckets = NULL;
	size_t const old_capacity = table->capacity;
	size_t new_capacity = table->capacity << log2_grow_factor;
	while (new_capacity < table->capacity)
	{
		/* integer overflow */
		--log2_grow_factor;
		new_capacity = old_capacity << log2_grow_factor;
	}
	if (new_capacity <= old_capacity)
		return;
		
	const size_t bucket_size = table->bucket_size;
	void* const opaque = table->opaque;

	buckets = GENC_CXX_CAST(char*, table->realloc_fn(
		table->buckets, old_capacity * bucket_size, new_capacity * bucket_size, opaque));
	if (!buckets)
		return;
	
	// zero out the newly added buckets
	for (genc_hash_t idx = old_capacity; idx < new_capacity; ++idx)
	{
		table->item_clear_fn(buckets + idx * bucket_size, opaque);
	}
	
	table->buckets = buckets;
	table->capacity = new_capacity;
	
	const genc_item_is_empty_fn item_empty_fn = table->item_empty_fn;
	
	// this is the fun/crazy part:
	for (genc_hash_t idx = 0; idx < new_capacity; ++idx)
	{
		char* bucket = buckets + idx * bucket_size;
		if (item_empty_fn(buckets + idx * bucket_size, opaque))
		{
			if (idx >= old_capacity)
				break; // no need to keep looking, as anything past the first empty bucket in the new buckets is already new
		}
		else if (genc_lpht_insert_item_into_table(table, bucket))
		{
			// item was moved
			table->item_clear_fn(bucket, opaque);
		}
	}
}

/* Walks all the elements in the hash table and checks they're still in the correct bucket. */
genc_bool_t genc_lpht_verify(struct genc_linear_probing_hash_table* table)
{
	const size_t capacity = table->capacity;
	const size_t bucket_size = table->bucket_size;
	const genc_item_is_empty_fn item_empty_fn = table->item_empty_fn;
	const genc_hash_get_item_key_fn get_key_fn = table->get_key_fn;
	void* const opaque = table->opaque;
	
	char* bucket = GENC_CXX_CAST(char*, table->buckets);
	for (genc_hash_t idx = 0; idx < capacity; ++idx, bucket += bucket_size)
	{
		if (!item_empty_fn(bucket, opaque))
		{
			void* key = get_key_fn(bucket, opaque);
			void* found = genc_lpht_find(table, key);
			if (found != bucket)
				return false;
		}
	}
	return true;
}

void* genc_lpht_first_item(struct genc_linear_probing_hash_table* table)
{
	const size_t capacity = table->capacity;
	const size_t bucket_size = table->bucket_size;
	const genc_item_is_empty_fn item_empty_fn = table->item_empty_fn;
	void* const opaque = table->opaque;

	char* bucket = GENC_CXX_CAST(char*, table->buckets);

	for (genc_hash_t idx = 0; idx < capacity; ++idx, bucket += bucket_size)
	{
		if (!item_empty_fn(bucket, opaque))
			return bucket;
	}
	return NULL;
}
/* Next non-empty bucket */
void* genc_lpht_next_item(struct genc_linear_probing_hash_table* table, void* cur_item)
{
	const size_t capacity = table->capacity;
	const size_t bucket_size = table->bucket_size;
	const genc_item_is_empty_fn item_empty_fn = table->item_empty_fn;
	void* const opaque = table->opaque;

	char* bucket = GENC_CXX_CAST(char*, cur_item);

	char* end = GENC_CXX_CAST(char*, table->buckets);
	end += capacity * bucket_size;

	for (; bucket < end; bucket += bucket_size)
	{
		if (!item_empty_fn(bucket, opaque))
			return bucket;
	}
	return NULL;
}

