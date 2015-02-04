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

static void clear_buckets(void* buckets, const size_t capacity, const size_t bucket_size, genc_item_clear_fn item_clear_fn, void* opaque);

void genc_lpht_clear(genc_linear_probing_hash_table_t* table)
{
	genc_lphtl_clear(&table->table, &table->desc, table->opaque);
}
void genc_lphtl_clear(genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque)
{
	clear_buckets(table->buckets, table->capacity, desc->bucket_size, desc->item_clear_fn, opaque);
	table->item_count = 0;
}

static void* alloc_empty_buckets(
	genc_realloc_fn realloc_fn, genc_item_clear_fn item_clear_fn,
	size_t capacity, size_t bucket_size, void* opaque)
{
	void* buckets = realloc_fn(NULL, 0, capacity * bucket_size, opaque);
	if (!buckets)
		return NULL;

	clear_buckets(buckets, capacity, bucket_size, item_clear_fn, opaque);
	return buckets;
}

static void clear_buckets(void* buckets, const size_t capacity, const size_t bucket_size, genc_item_clear_fn item_clear_fn, void* opaque)
{	// set all buckets to empty
	char* bucket = GENC_CXX_CAST(char*, buckets);
	for (size_t i = 0; i < capacity; ++i)
	{
		item_clear_fn(bucket, opaque);
		bucket += bucket_size;
	}
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
	genc_linear_probing_hash_table_desc_init(
		&table->desc,
		hash_fn, get_key_fn, key_equality_fn, item_empty_fn, item_clear_fn, realloc_fn,
		bucket_size, load_percent_grow_threshold, load_percent_shrink_threshold);
	table->opaque = opaque;
	return genc_linear_probing_hash_table_light_init(&table->table, &table->desc, opaque, initial_capacity_pow2);
}

genc_bool_t genc_linear_probing_hash_table_light_init(
	genc_linear_probing_hash_table_light_t* table,
	const genc_linear_probing_hash_table_desc_t* desc,
	void* opaque,
	size_t initial_capacity_pow2)
{
	void* buckets;
	if (!genc_is_pow2(initial_capacity_pow2))
		return 0;
	if (SIZE_MAX / desc->bucket_size < initial_capacity_pow2)
		return 0; // overflow
	
	buckets = alloc_empty_buckets(
		desc->realloc_fn, desc->item_clear_fn, initial_capacity_pow2, desc->bucket_size, opaque);
	if (!buckets)
		return false;
	
	table->capacity = initial_capacity_pow2;
	table->item_count = 0;
	table->buckets = buckets;
	return true;
}

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
	uint8_t load_percent_shrink_threshold)
{
	desc->hash_fn = hash_fn;
	desc->get_key_fn = get_key_fn;
	desc->key_equality_fn = key_equality_fn;
	desc->item_empty_fn = item_empty_fn;
	desc->item_clear_fn = item_clear_fn;
	desc->realloc_fn = realloc_fn;
	desc->bucket_size = bucket_size;
	desc->load_percent_grow_threshold = load_percent_grow_threshold;
	desc->load_percent_shrink_threshold = load_percent_shrink_threshold;
}

/* Returns the current number of items in the hash table. */
size_t genc_lpht_count(struct genc_linear_probing_hash_table* table)
{
	return table->table.item_count;
}
size_t genc_lphtl_count(genc_linear_probing_hash_table_light_t* table)
{
	return table->item_count;
}

/* Returns the number of buckets allocated. */
size_t genc_lpht_capacity(struct genc_linear_probing_hash_table* table)
{
	return table->table.capacity;
}
size_t genc_lphtl_capacity(genc_linear_probing_hash_table_light_t* table)
{
	return table->capacity;
}

/* Drops all items from the table and deallocates bucket array memory. */
void genc_lpht_destroy(struct genc_linear_probing_hash_table* table)
{
	genc_lphtl_destroy(&table->table, &table->desc, table->opaque);
}
void genc_lphtl_destroy(genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque)
{
	if (table->buckets)
	{
		desc->realloc_fn(table->buckets, table->capacity * desc->bucket_size, 0, opaque);
		table->buckets = NULL;
		table->capacity = 0;
		table->item_count = 0;
	}
}

void genc_lphtl_zero(genc_linear_probing_hash_table_light_t* table)
{
	table->buckets = NULL;
	table->capacity = table->item_count = 0;
}

/* locates the bucket which either matches key or which we can insert an item
 * with that key into */
static void* genc_lphtl_find_or_empty(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* key, bool* out_found)
{
	genc_hash_t start_idx = genc_lphtl_get_bucket_for_key(table, desc, opaque, key);
	
	*out_found = false;
	genc_hash_t idx = start_idx;
	do
	{
		char* bucket = GENC_CXX_CAST(char*, table->buckets);
		bucket += desc->bucket_size * idx;
		
		// stop if bucket is empty or if we found an item which matches key
		if (desc->item_empty_fn(bucket, opaque))
			return bucket;
		void* bucket_key = desc->get_key_fn(bucket, opaque);
		if (desc->key_equality_fn(bucket_key, key, opaque))
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
static void* genc_lphtl_insert_item_into_table(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque, void* item)
{
	void* item_key = desc->get_key_fn(item, opaque);
	bool found = false;
	void* bucket = genc_lphtl_find_or_empty(table, desc, opaque, item_key, &found);
	
	if (!bucket || found)
		return NULL; // table is full, or item exists
	
	// insert the item
	memcpy(bucket, item, desc->bucket_size);
	return bucket;
}

// pure insertion/replacement, without the bookkeeping
static void* genc_lphtl_insert_or_replace_item_in_table(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque, void* item, genc_bool_t* out_replaced_existing)
{
	void* item_key = desc->get_key_fn(item, opaque);
	void* bucket = genc_lphtl_find_or_empty(table, desc, opaque, item_key, out_replaced_existing);
	
	if (!bucket)
		return NULL; // table is full
	
	// insert/replace the item
	memcpy(bucket, item, desc->bucket_size);
	return bucket;
}

/* Inserts the given item into the hash table.
 * Returns NULL to report failure due to a duplicate or growth failure, pointer
 * to inserted bucket on success. */
void* genc_lpht_insert_item(
	struct genc_linear_probing_hash_table* table, void* item)
{
	return genc_lphtl_insert_item(&table->table, &table->desc, table->opaque, item);
}

void* genc_lpht_insert_or_update_item(
	struct genc_linear_probing_hash_table* table, void* item)
{
	return genc_lphtl_insert_or_update_item(&table->table, &table->desc, table->opaque, item);
}

bool genc_lphtl_reserve_space(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque, size_t target_count)
{
	unsigned new_load = 100;
	
	if (table->capacity > 0)
		new_load = (unsigned)(100ul * target_count / table->capacity);
	if (new_load > desc->load_percent_grow_threshold || new_load >= 100)
	{
		int factor_log2 = genc_log2_size(new_load / desc->load_percent_grow_threshold);
		if (new_load > ((unsigned)desc->load_percent_grow_threshold) << factor_log2)
			++factor_log2;
		
		/*printf("New load factor %d%%, growth threshold reached. Growing by factor 1 << %d (%u).\n", new_load, factor_log2, 1u << factor_log2);*/
		return genc_lphtl_grow_by(table, desc, opaque, factor_log2);
	}
	return true;
}

void* genc_lphtl_insert_item(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* item)
{
	if (!item) return NULL;
	
	genc_lphtl_reserve_space(table, desc, opaque, table->item_count + 1);
	
	void* inserted = genc_lphtl_insert_item_into_table(table, desc, opaque, item);
	if (!inserted)
		return NULL;
	
	++table->item_count;
	return inserted;
}

void* genc_lphtl_insert_or_update_item(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* item)
{
	if (!item) return NULL;
	
	genc_lphtl_reserve_space(table, desc, opaque, table->item_count + 1);
	
	bool updated_existing = false;
	void* inserted = genc_lphtl_insert_or_replace_item_in_table(table, desc, opaque, item, &updated_existing);
	if (!inserted)
		return NULL;
	
	if (!updated_existing)
		++table->item_count;
	return inserted;
}

genc_lpht_insertion_test_result_t genc_lpht_can_insert_item(
	struct genc_linear_probing_hash_table* table, void* item)
{
	return genc_lphtl_can_insert_item(&table->table, &table->desc, table->opaque, item);
}

genc_lpht_insertion_test_result_t genc_lphtl_can_insert_item(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* item)
{
	genc_lpht_insertion_test_result_t res = { GENC_LPHT_INSERT_NULL, 0};
	unsigned new_load = 100;
	if (!item) return res;

	if (table->capacity > 0)
		new_load = (unsigned)(100ul * (table->item_count + 1ul) / table->capacity);
	if (new_load > desc->load_percent_grow_threshold || new_load >= 100)
	{
		int factor_log2 = genc_log2_size(new_load / desc->load_percent_grow_threshold);
		if (new_load > ((unsigned)desc->load_percent_grow_threshold) << factor_log2)
			++factor_log2;
		
		size_t const old_capacity = table->capacity;
		size_t new_capacity = table->capacity << factor_log2;
		while (new_capacity < table->capacity)
		{
			/* integer overflow */
			--factor_log2;
			new_capacity = old_capacity << factor_log2;
		}
		if (new_capacity <= old_capacity)
		{
			if (table->item_count >= table->capacity)
				res.type = GENC_LPHT_INSERT_SIZE_OVERFLOW;
			else
				res.type = GENC_LPHT_INSERT_SIMPLE;
		}
		else
		{
			res.resize_bytes = new_capacity * desc->bucket_size;
			res.type =
				(table->item_count >= table->capacity)
				? GENC_LPHT_INSERT_NEEDS_RESIZE : GENC_LPHT_INSERT_WANTS_RESIZE;
		}
		return res;
	}
	res.type = GENC_LPHT_INSERT_SIMPLE;
	return res;
}


/* Looks up the key in the table, returning the matching item if present, or NULL otherwise. */
void* genc_lpht_find(struct genc_linear_probing_hash_table* table, void* key)
{
	return genc_lphtl_find(&table->table, &table->desc, table->opaque, key);
}
void* genc_lphtl_find(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* key)
{
	bool found = false;
	void* bucket = genc_lphtl_find_or_empty(table, desc, opaque, key, &found);
	return found ? bucket : NULL;
}

/* Hashes the key and returns the bucket into which the key falls */
genc_hash_t genc_lpht_get_bucket_for_key(
	struct genc_linear_probing_hash_table* table, void* key)
{
	return genc_lphtl_get_bucket_for_key(&table->table, &table->desc, table->opaque, key);
}
genc_hash_t genc_lphtl_get_bucket_for_key(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* key)
{
	genc_hash_t hash = desc->hash_fn(key, opaque);
	hash &= (table->capacity - 1ul);
	return hash;
}

static GENC_INLINE genc_bool_t idx_between(
	genc_hash_t idx, genc_hash_t start_inc, genc_hash_t end_exc, genc_hash_t capacity)
{
	genc_hash_t mask = capacity - 1;
	// calculate distances idx->end_exc and start_exc->end_exc using modular arithmetic
	genc_hash_t idx_delta = (end_exc - idx) & mask;
	genc_hash_t start_delta = (end_exc - start_inc) & mask;
	
	return idx_delta <= start_delta;
}

/* Removes the item from the hash table.
 * Deallocation, like allocation, is the responsibility of the caller.
 */
void genc_lpht_remove(struct genc_linear_probing_hash_table* table, void* item)
{
	genc_lphtl_remove(&table->table, &table->desc, table->opaque, item);
}
void genc_lphtl_remove(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* opaque,
	void* item)
{
	if (item && !desc->item_empty_fn(item, opaque))
	{
		desc->item_clear_fn(item, opaque);
		--table->item_count;
		
		const size_t bucket_size = desc->bucket_size;
		genc_item_clear_fn item_clear_fn = desc->item_clear_fn;
		genc_item_is_empty_fn item_empty_fn = desc->item_empty_fn;
		const genc_hash_get_item_key_fn get_key_fn = desc->get_key_fn;
		
		// need to move up any displaced items which would now be unreachable
		char* buckets = GENC_CXX_CAST(char*, table->buckets);
		char* empty_bucket = GENC_CXX_CAST(char*, item);
		genc_hash_t start_idx = (empty_bucket - buckets) / bucket_size;
		genc_hash_t empty_idx = start_idx;
		genc_hash_t idx = (start_idx + 1) & (table->capacity - 1);
		/* all consecutive non-empty buckets are reachable, so keep going until we
		 * find an empty one. */
		char* bucket = buckets + idx * bucket_size;
		while (!item_empty_fn(bucket, opaque))
		{
			/* If current item is not reachable from its true slot, we need to move it
			 * into the empty one */
			void* item_key = get_key_fn(bucket, opaque);
			genc_hash_t key_bucket_idx = genc_lphtl_get_bucket_for_key(table, desc, opaque, item_key);
			if (idx_between(empty_idx, key_bucket_idx, idx, table->capacity))
			{
				memcpy(empty_bucket, bucket, bucket_size);
				item_clear_fn(bucket, opaque);
				empty_idx = idx;
				empty_bucket = bucket;
			}
			
			++idx;
			idx &= (table->capacity - 1);
			bucket = buckets + idx * bucket_size;
		}

		// shrink if necessary
		unsigned new_load = 0;
		if (table->capacity > 0)
			new_load = (unsigned)(100ull * (table->item_count) / table->capacity);

		if (new_load > 0 && new_load < desc->load_percent_shrink_threshold)
		{
			int factor_log2 = genc_log2_size(desc->load_percent_shrink_threshold / new_load);
			genc_lphtl_shrink_by(table, desc, opaque, factor_log2);
		}
	}
}

/* Shrink the capacity of the table by a factor of 1 << log2_shrink_factor */
bool genc_lpht_shrink_by(struct genc_linear_probing_hash_table* table, unsigned log2_shrink_factor)
{
	return genc_lphtl_shrink_by(&table->table, &table->desc, table->opaque, log2_shrink_factor);
}
bool genc_lphtl_shrink_by(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void*const opaque,
	unsigned log2_shrink_factor)
{
	const size_t old_capacity = table->capacity;
	// don't shrink it down so far that the contents no longer fits
	while (table->item_count > (old_capacity >> log2_shrink_factor))
	{
		if (log2_shrink_factor < 1)
			return false;
		--log2_shrink_factor;
	}
	
	const size_t bucket_size = desc->bucket_size;
	
	// TODO: resize in-place
	const size_t new_capacity = old_capacity >> log2_shrink_factor;
	
	// alloc new bucket array
	const genc_realloc_fn realloc_fn = desc->realloc_fn;
	char* new_buckets = GENC_CXX_CAST(
		char*, alloc_empty_buckets(realloc_fn, desc->item_clear_fn, new_capacity, bucket_size, opaque));
	if (!new_buckets)
		return false;
	
	// re-insert items into new buckets
	char* old_buckets = GENC_CXX_CAST(char*, table->buckets);
	table->buckets = new_buckets;
	table->capacity = new_capacity;
	
	const genc_item_is_empty_fn item_empty_fn = desc->item_empty_fn;
	char* old_bucket = old_buckets;
	for (genc_hash_t idx = 0; idx < old_capacity; ++idx, old_bucket += bucket_size)
	{
		if (!item_empty_fn(old_bucket, opaque)
		    && !genc_lphtl_insert_item_into_table(table, desc, opaque, old_bucket))
		{
			// failed to move item across, give up
			table->buckets = old_buckets;
			table->capacity = old_capacity;
			realloc_fn(
				new_buckets, new_capacity * bucket_size, 0, opaque);
			return false;
		}
	}
	
	realloc_fn(
		old_buckets, old_capacity * bucket_size, 0, opaque);
	return true;
}

/* Grow the capacity of the table by a factor of 1 << log2_grow_factor */
bool genc_lpht_grow_by(
	struct genc_linear_probing_hash_table* table, unsigned log2_grow_factor)
{
	return genc_lphtl_grow_by(&table->table, &table->desc, table->opaque, log2_grow_factor);
}
bool genc_lphtl_grow_by(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* const opaque,
	unsigned log2_grow_factor)
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
		return false;
		
	const size_t bucket_size = desc->bucket_size;

	buckets = GENC_CXX_CAST(char*, desc->realloc_fn(
		table->buckets, old_capacity * bucket_size, new_capacity * bucket_size, opaque));
	if (!buckets)
		return false;
	
	// zero out the newly added buckets
	const genc_item_clear_fn item_clear_fn = desc->item_clear_fn;
	for (genc_hash_t idx = old_capacity; idx < new_capacity; ++idx)
	{
		item_clear_fn(buckets + idx * bucket_size, opaque);
	}
	
	table->buckets = buckets;
	table->capacity = new_capacity;
	
	const genc_item_is_empty_fn item_empty_fn = desc->item_empty_fn;
	
	// this is the fun/crazy part:
	for (genc_hash_t idx = 0; idx < new_capacity; ++idx)
	{
		char* bucket = buckets + idx * bucket_size;
		if (item_empty_fn(buckets + idx * bucket_size, opaque))
		{
			if (idx >= old_capacity)
				break; // no need to keep looking, as anything past the first empty bucket in the new buckets is already new
		}
		else if (genc_lphtl_insert_item_into_table(table, desc, opaque, bucket))
		{
			// item was moved
			item_clear_fn(bucket, opaque);
		}
	}
	return true;
}

bool genc_lpht_resize(struct genc_linear_probing_hash_table* table, size_t new_capacity)
{
	if (new_capacity < table->table.item_count)
		return false;
	else if (new_capacity < table->table.capacity)
		return genc_lpht_shrink_by(table, genc_log2_size(table->table.capacity / new_capacity));
	else if (new_capacity > table->table.capacity)
		return genc_lpht_grow_by(table, genc_log2_size(new_capacity / table->table.capacity));
	else
		return true; // nothing to do (new capacity = old)
}

static genc_bool_t genc_lphtl_verify(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* const opaque);
/* Walks all the elements in the hash table and checks they're still in the correct bucket. */
genc_bool_t genc_lpht_verify(struct genc_linear_probing_hash_table* table)
{
	return genc_lphtl_verify(&table->table, &table->desc, table->opaque);
}
static genc_bool_t genc_lphtl_verify(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* const opaque)
{
	const size_t capacity = table->capacity;
	const size_t bucket_size = desc->bucket_size;
	const genc_item_is_empty_fn item_empty_fn = desc->item_empty_fn;
	const genc_hash_get_item_key_fn get_key_fn = desc->get_key_fn;
	
	char* bucket = GENC_CXX_CAST(char*, table->buckets);
	for (genc_hash_t idx = 0; idx < capacity; ++idx, bucket += bucket_size)
	{
		if (!item_empty_fn(bucket, opaque))
		{
			void* key = get_key_fn(bucket, opaque);
			void* found = genc_lphtl_find(table, desc, opaque, key);
			if (found != bucket)
				return false;
		}
	}
	return true;
}

void* genc_lpht_first_item(struct genc_linear_probing_hash_table* table)
{
	return genc_lphtl_first_item(&table->table, &table->desc, table->opaque);
}
void* genc_lphtl_first_item(genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* const opaque)
{
	const size_t capacity = table->capacity;
	const size_t bucket_size = desc->bucket_size;
	const genc_item_is_empty_fn item_empty_fn = desc->item_empty_fn;

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
	return genc_lphtl_next_item(&table->table, &table->desc, table->opaque, cur_item);
}
void* genc_lphtl_next_item(
	genc_linear_probing_hash_table_light_t* table, const genc_linear_probing_hash_table_desc_t* desc, void* const opaque,
	void* cur_item)
{
	const size_t capacity = table->capacity;
	const size_t bucket_size = desc->bucket_size;
	const genc_item_is_empty_fn item_empty_fn = desc->item_empty_fn;

	char* bucket = GENC_CXX_CAST(char*, cur_item);

	char* end = GENC_CXX_CAST(char*, table->buckets);
	end += capacity * bucket_size;

	bucket += bucket_size;
	for (; bucket < end; bucket += bucket_size)
	{
		if (!item_empty_fn(bucket, opaque))
			return bucket;
	}
	return NULL;
}

