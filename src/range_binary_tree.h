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

#ifndef ssdcache_range_binary_tree_h
#define ssdcache_range_binary_tree_h

#include "binary_tree.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct genc_range_binary_tree_item
{
	genc_bt_node_head_t head;
	/*binary tree key*/
	uint64_t range_start;
	/*non inclusive*/
	uint64_t range_end;
}; 
typedef struct genc_range_binary_tree_item genc_range_binary_tree_item_t;
 
/* Structure for returning a range of tree items, inclusive of start, not including end. */
struct genc_range_bt_node_range
{
	genc_range_binary_tree_item_t* start;
	genc_range_binary_tree_item_t* end;
};
typedef struct genc_range_bt_node_range genc_range_bt_node_range_t;


void genc_range_binary_tree_init(genc_binary_tree_t* tree);

genc_range_bt_node_range_t genc_range_bt_find_overlap(genc_binary_tree_t* tree, genc_range_binary_tree_item_t* range);

/* Inserts a new range into the tree if it doesn't overlap any existing nodes,
 * returning 1 on success. 0 is returned if the range wasn't added because there
 * is overlap. */
int genc_range_bt_insert(genc_binary_tree_t* tree, genc_range_binary_tree_item_t* new_range);

struct genc_range_bt_chop_result
{
	int did_split;
	/* The removed nodes as a linked list along the 'right' pointer, in ascending order */
	genc_bt_node_head_t* removed_node_list;
	
	/* The node which straddled the start of the range and was truncated or split (or NULL, if no such node) */
	genc_range_binary_tree_item_t* start_truncated;
	/* The node which straddled the end of the range, or the newly inserted split item, if there was a split */
	genc_range_binary_tree_item_t* end_truncated_or_split;
};
typedef struct genc_range_bt_chop_result genc_range_bt_chop_result_t;

genc_range_bt_chop_result_t genc_range_bt_chop_range(
	genc_binary_tree_t* tree, genc_range_binary_tree_item_t* range, genc_range_binary_tree_item_t* split_item);

#ifdef __cplusplus
}
#endif

#endif
