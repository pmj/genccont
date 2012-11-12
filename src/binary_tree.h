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

#ifndef GENCCONT_BINARY_TREE_H
#define GENCCONT_BINARY_TREE_H

#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct genc_bt_node_head;
struct genc_binary_tree;
struct genc_bt_iterator;

typedef struct genc_binary_tree genc_binary_tree_t;
typedef struct genc_bt_node_head genc_bt_node_head_t;

/* Each element in a binary tree must contain such a structure. Automatically
 * initialised upon insertion into a tree. */
struct genc_bt_node_head
{
	genc_bt_node_head_t* parent;
	genc_bt_node_head_t* left;
	genc_bt_node_head_t* right;
};

/* Must return true(1) if a should appear before b, false(0) otherwise.
 * To test for equality, the function will be called with
 * reversed arguments if the first call returns false. The
 * second call must return false as well if the items are equal.
 * The opaque pointer can be used to affect the comparison behaviour on a
 * per-tree basis and will match the pointer passed to genc_binary_tree_init().
 */
typedef genc_bool_t(*genc_binary_tree_less_fn)(genc_bt_node_head_t* a, genc_bt_node_head_t* b, void* opaque);

struct genc_binary_tree
{
	genc_bt_node_head_t* root;
	genc_bt_node_head_t* min_node;
	genc_bt_node_head_t* max_node;
	genc_binary_tree_less_fn less_fn;
	void* less_fn_opaque;
};

/* Initialise a blank binary tree, using the specified comparison function */
void genc_binary_tree_init(genc_binary_tree_t* tree, genc_binary_tree_less_fn less_fn, void* less_fn_opaque);
/* Insert an item into the tree by searching the tree to find the appropriate
 * location. Returns true (1) on success or false (0) if an equal item is already present. */
genc_bool_t genc_bt_insert(genc_binary_tree_t* tree, genc_bt_node_head_t* item);
/* Remove the given item from the tree. */
void genc_bt_remove(genc_binary_tree_t* tree, genc_bt_node_head_t* item);
/* Search the tree for an item value and return the reference pointing to the
 * matching item if found, or the child reference where such an item would be
 * inserted. Also returns the parent node of said reference in both cases,
 * or NULL if the root reference is returned.
 * */
genc_bt_node_head_t** genc_bt_find_insertion_point(genc_binary_tree_t* tree, genc_bt_node_head_t* item, genc_bt_node_head_t** out_parent);
/* Returns the tree node equal to item, or NULL if no such node exists. */
genc_bt_node_head_t* genc_bt_find(genc_binary_tree_t* tree, genc_bt_node_head_t* item);
/* Tries to find a node in the tree with the greatest key less than or equal to
 * item's. If the tree contains no such node, NULL is returned. */
genc_bt_node_head_t* genc_bt_find_or_lower(genc_binary_tree_t* tree, genc_bt_node_head_t* item); 
/* Tries to find a node in the tree with the smallest key greater than or equal to
 * item's. If the tree contains no such node, NULL is returned. */
genc_bt_node_head_t* genc_bt_find_or_higher(genc_binary_tree_t* tree, genc_bt_node_head_t* item);

/*genc_bt_node_head_t* genc_bt_find_or_higher(genc_binary_tree_t* tree, genc_bt_node_head_t* item);*/
genc_bt_node_head_t* genc_bt_first_item(genc_binary_tree_t* tree);
genc_bt_node_head_t* genc_bt_next_item(genc_binary_tree_t* tree, genc_bt_node_head_t* after_item);
genc_bt_node_head_t* genc_bt_last_item(genc_binary_tree_t* tree);
genc_bt_node_head_t* genc_bt_prev_item(genc_binary_tree_t* tree, genc_bt_node_head_t* before_item);

void genc_bt_swap_trees(genc_binary_tree_t* tree_a, genc_binary_tree_t* tree_b);
genc_bool_t genc_bt_is_empty(genc_binary_tree_t* tree);

#ifdef __cplusplus
} /* extern "C" */
#endif

#define genc_bt_find_obj(tree, item, type, member) \
	genc_container_of(genc_bt_find(tree, &(item)->member), type, member)

#define genc_bt_find_obj_or_lower(tree, item, type, member) \
	genc_container_of(genc_bt_find_or_lower(tree, &(item)->member), type, member)

#define genc_bt_find_obj_or_higher(tree, item, type, member) \
	genc_container_of(genc_bt_find_or_higher(tree, &(item)->member), type, member)

#define genc_bt_next_obj(tree, item, type, member) \
	genc_container_of(genc_bt_next_item(tree, &(item)->member), type, member)

#define genc_bt_prev_obj(tree, item, type, member) \
	genc_container_of(genc_bt_prev_item(tree, &(item)->member), type, member)

#define genc_bt_first_obj(tree, type, member) \
	genc_container_of(genc_bt_first_item(tree), type, member)

#define genc_bt_last_obj(tree, type, member) \
	genc_container_of(genc_bt_last_item(tree), type, member)

#endif
