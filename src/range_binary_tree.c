/*
 Copyright (c) 2011-2012 Phil Jordan <phil@philjordan.eu>
 
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

#include "range_binary_tree.h"

#ifndef KERNEL
#include <assert.h>
#endif

static genc_bool_t range_node_less(genc_bt_node_head_t* a_head, genc_bt_node_head_t* b_head, void* opaque GENC_UNUSED)
{
	genc_range_binary_tree_item_t* a =
		genc_container_of_notnull(a_head, genc_range_binary_tree_item_t, head);
	genc_range_binary_tree_item_t* b =
		genc_container_of_notnull(b_head, genc_range_binary_tree_item_t, head);
	if (a->range_start < b->range_start) 
	{
		return 1;
	}
	return 0;
}

/* Tests if two ranges overlap, and returns 0 if there is any overlap.
 * -1 is returned if all of range a is below range b, and 1 is returned if all
 * of range a falls above range b. */
static int compare_ranges(genc_range_binary_tree_item_t* a, genc_range_binary_tree_item_t* b)
{
	if (a->range_end <= b->range_start)
		return -1;
	else if (a->range_start >= b->range_end)
		return 1;
	return 0;
}

int genc_range_binary_tree_compare_ranges(genc_range_binary_tree_item_t* a, genc_range_binary_tree_item_t* b)
{
	return compare_ranges(a, b);
}


void genc_range_binary_tree_init(genc_binary_tree_t* tree)
{
	genc_binary_tree_init(tree, range_node_less, NULL);
}

genc_range_bt_node_range_t genc_range_bt_find_overlap(genc_binary_tree_t* tree, genc_range_binary_tree_item_t* range)
{
	/* Find the highest range starting at or before the start of the range we're testing against */
	genc_range_binary_tree_item_t* found = genc_bt_find_obj_or_lower(tree, range, genc_range_binary_tree_item_t, head);
	if (!found)
	{
		/* No range is lower, so test the first */
		found = genc_bt_first_obj(tree, genc_range_binary_tree_item_t, head);
	
		if (!found)
		{
			/* tree is empty */
			genc_range_bt_node_range_t empty = { NULL, NULL };
			return empty;
		}
	}
	
	/* Find the first range that overlaps ours */
	int cmp = compare_ranges(found, range);
	if (cmp < 0)
	{
		/* found range lies entirely below ours, move to next item, which can't */
		found = genc_bt_next_obj(tree, found, genc_range_binary_tree_item_t, head);
	}
	
	/* found must now overlap, lie above our range, or be NULL */
	genc_range_bt_node_range_t overlap = { found, found };
	
	/* Keep going until we fall off the end of the range */
	while (found && (0 == compare_ranges(found, range)))
	{
		found = genc_bt_next_obj(tree, found, genc_range_binary_tree_item_t, head);
	}
	overlap.end = found;
	return overlap;
}

genc_bool_t genc_range_bt_insert(genc_binary_tree_t* tree, genc_range_binary_tree_item_t* new_range)
{
	genc_range_bt_node_range_t overlap = genc_range_bt_find_overlap(tree, new_range);
	if (overlap.start != overlap.end)
		return 0;
	return genc_bt_insert(tree, &new_range->head);
}

void genc_range_bt_split_range(
	genc_binary_tree_t* tree, genc_range_binary_tree_item_t* existing_range,
	uint64_t split_at, genc_range_binary_tree_item_t* new_range)
{
	assert(split_at > existing_range->range_start && split_at < existing_range->range_end);
	
	new_range->range_end = existing_range->range_end;
	new_range->range_start = split_at;
	existing_range->range_end = split_at;
	
	int ok GENC_UNUSED = genc_range_bt_insert(tree, new_range);
	assert(ok);
}

genc_range_bt_chop_result_t genc_range_bt_chop_range(
	genc_binary_tree_t* tree, genc_range_binary_tree_item_t* range, genc_range_binary_tree_item_t* split_item)
{
	genc_range_bt_chop_result_t result = { 0, NULL, NULL, NULL };
	genc_range_bt_node_range_t overlap = genc_range_bt_find_overlap(tree, range);
	if (overlap.start != overlap.end)
	{
		genc_range_binary_tree_item_t* cur = overlap.start;
		if (cur->range_start < range->range_start)
		{
			/* partial overlap at beginning, need to truncate or split this element */
			result.start_truncated = cur;
			if (cur->range_end > range->range_end)
			{
				/* This must have been the only overlapping node */
				assert(overlap.end == genc_bt_next_obj(tree, cur, genc_range_binary_tree_item_t, head));

				/* Element needs to be split */
				assert(split_item);
				split_item->range_start = range->range_end;
				split_item->range_end = cur->range_end;
				cur->range_end = range->range_start;

				/* Insert the new, split item. */
				genc_range_bt_insert(tree, split_item);
				result.did_split = 1;
				result.end_truncated_or_split = split_item;
				/* no nodes removed */
				return result;
			}
			
			/* shrink this element, then move to the next one */
			cur->range_end = range->range_start;
			cur = genc_bt_next_obj(tree, cur, genc_range_binary_tree_item_t, head);
		}
		
		genc_bt_node_head_t** removed_list_tail = &result.removed_node_list;
		
		while (cur != overlap.end && cur->range_end <= range->range_end)
		{
			genc_range_binary_tree_item_t* next = genc_bt_next_obj(tree, cur, genc_range_binary_tree_item_t, head);
			/* element falls entirely within the search range, remove it */
			genc_bt_remove(tree, &cur->head);

			*removed_list_tail = &cur->head;
			removed_list_tail = &cur->head.right;
			
			cur = next;
		}
		
		if (cur != overlap.end)
		{
			/* this node overlaps the end of our chop range, shrink it */
			cur->range_start = range->range_end;
			result.end_truncated_or_split = cur;
			
			/* must be the last overlapping node! */
			cur = genc_bt_next_obj(tree, cur, genc_range_binary_tree_item_t, head);
			assert(cur == overlap.end);
		}
	}
	return result;
}
