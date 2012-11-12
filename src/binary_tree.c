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

#include "binary_tree.h"
#ifndef KERNEL
#include <assert.h>
#endif

static genc_bt_node_head_t* genc_bt_rightmost_in_subtree(genc_bt_node_head_t* subtree);
static genc_bt_node_head_t* genc_bt_leftmost_in_subtree(genc_bt_node_head_t* subtree);

void genc_binary_tree_init(genc_binary_tree_t* tree, genc_binary_tree_less_fn less_fn, void* less_fn_opaque)
{
	tree->root = tree->min_node = tree->max_node = NULL;
	tree->less_fn = less_fn;
	tree->less_fn_opaque = less_fn_opaque;
}

genc_bt_node_head_t** genc_bt_find_insertion_point(genc_binary_tree_t* tree, genc_bt_node_head_t* item, genc_bt_node_head_t** out_parent)
{
	genc_bt_node_head_t** child_ref = &tree->root;
	genc_binary_tree_less_fn less = tree->less_fn;
	genc_bt_node_head_t* child;
	*out_parent = NULL;
	while ((child = *child_ref))
	{
		*out_parent = child;
		if (less(item, child, tree->less_fn_opaque))
		{
			child_ref = &child->left;
		}
		else if (less(child, item, tree->less_fn_opaque))
		{
			child_ref = &child->right;
		}
		else
		{
			*out_parent = child->parent;
			return child_ref;
		}
	}
	return child_ref;
}

genc_bool_t genc_bt_insert(genc_binary_tree_t* tree, genc_bt_node_head_t* item)
{
	item->left = NULL;
	item->right = NULL;
	if (!tree->root)
	{
		/* Inserting into empty tree. */
		tree->root = tree->min_node = tree->max_node = item;
		item->parent = NULL;
		return 1;
	}
			
	genc_bt_node_head_t* parent = NULL;
	genc_bt_node_head_t** ins = genc_bt_find_insertion_point(tree, item, &parent);
	if (*ins)
		return 0; /* equal item already exists */
		
	/* do insertion */
	item->parent = parent;
	*ins = item;
	
	if (parent == tree->min_node && ins == &parent->left)
	{
		/* inserting to the left of the left-most node means we become the new left-most node. */
		tree->min_node = item;
	}
	if (parent == tree->max_node && ins == &parent->right)
	{
		/* inserting to the right of the right-most node means we become the new right-most node. */
		tree->max_node = item;
	}
	
	return 1;
}

void genc_bt_remove(genc_binary_tree_t* tree, genc_bt_node_head_t* item)
{
	genc_bt_node_head_t* replacement = NULL;
	genc_bt_node_head_t** parent_child_ref = NULL;
	if (item->left)
	{
		if (item->right)
		{
			/* removal of a 2-child node means we need to find the next or previous node
			 * as the replacement node. For now, always use the next. */
			replacement = genc_bt_next_item(tree, item);
			/* by definition, the next item will not have a 'left' child, so its removal
			 * will be trivial */
			genc_bt_remove(tree, replacement);

			replacement->left = item->left;
			item->left->parent = replacement;
			replacement->right = item->right;
			if (item->right) /* the old item->right may be the replacement, and if so will have been removed */
				item->right->parent = replacement;
		}
		else
		{
			replacement = item->left;
		}
	}
	else if (item->right)
	{
		replacement = item->right;
	}

	if (replacement)
		replacement->parent = item->parent;

	if (item->parent)
	{
		if (item->parent->left == item)
			parent_child_ref = &item->parent->left;
		else
		{
			assert(item->parent->right == item);
			parent_child_ref = &item->parent->right;
		}
	}
	else
	{
		assert(item == tree->root);
		parent_child_ref = &tree->root;
	}
	
	*parent_child_ref = replacement;
	
	if (item == tree->max_node)
		tree->max_node = replacement ? genc_bt_rightmost_in_subtree(replacement) : item->parent;
	if (item == tree->min_node)
		tree->min_node = replacement ? genc_bt_leftmost_in_subtree(replacement) : item->parent;

	item->parent = NULL;
	item->left = NULL;
	item->right = NULL;
}


genc_bt_node_head_t* genc_bt_find(genc_binary_tree_t* tree, genc_bt_node_head_t* item)
{
	genc_bt_node_head_t* parent_unused = NULL;
	genc_bt_node_head_t** ref = genc_bt_find_insertion_point(tree, item, &parent_unused);
	return *ref;
}

/*
genc_bt_node_head_t* genc_bt_find_or_lower(genc_binary_tree_t* tree, genc_bt_node_head_t* item)
{
	
}
genc_bt_node_head_t* genc_bt_find_or_higher(genc_binary_tree_t* tree, genc_bt_node_head_t* item)
{
	
}
 */
genc_bt_node_head_t* genc_bt_first_item(genc_binary_tree_t* tree)
{
	return tree->min_node;
}

static genc_bt_node_head_t* genc_bt_leftmost_in_subtree(genc_bt_node_head_t* subtree)
{
	genc_bt_node_head_t* item = subtree;
	while (item->left)
		item = item->left;
	return item;
}

genc_bt_node_head_t* genc_bt_next_item(genc_binary_tree_t* tree, genc_bt_node_head_t* after_item)
{
	genc_bt_node_head_t* item;
	if ((item = after_item->right))
	{
		/* the next item is a descendant */
		return genc_bt_leftmost_in_subtree(item);
	}
	
	/* the next item will either be a parent or doesn't exist (we're at the end of the tree) */
	item = after_item;
	if (item == tree->max_node)
		return NULL;
	/* Walk up the tree until we find a node of which we're part of the left subtree */
	while (1)
	{
		genc_bt_node_head_t* parent = item->parent;
		if (!parent)
			return NULL;
		if (parent->left == item)
			return parent;
		item = parent;
	}
}
genc_bt_node_head_t* genc_bt_last_item(genc_binary_tree_t* tree)
{
	return tree->max_node;
}
static genc_bt_node_head_t* genc_bt_rightmost_in_subtree(genc_bt_node_head_t* subtree)
{
	genc_bt_node_head_t* item = subtree;
	while (item->right)
		item = item->right;
	return item;
}
genc_bt_node_head_t* genc_bt_prev_item(genc_binary_tree_t* tree, genc_bt_node_head_t* after_item)
{
	genc_bt_node_head_t* item;
	if ((item = after_item->left))
	{
		/* the next item is a descendant */
		return genc_bt_rightmost_in_subtree(item);
	}
	
	/* the next item will either be a parent or doesn't exist (we're at the start of the tree) */
	item = after_item;
	if (item == tree->min_node)
		return NULL;
	/* Walk up the tree until we find a node of which we're part of the right subtree */
	while (1)
	{
		genc_bt_node_head_t* parent = item->parent;
		if (!parent)
			return NULL;
		if (parent->right == item)
			return parent;
		item = parent;
	}
}

genc_bt_node_head_t* genc_bt_find_or_lower(genc_binary_tree_t* tree, genc_bt_node_head_t* item)
{
	genc_bt_node_head_t* parent = NULL;
	genc_bt_node_head_t** found = genc_bt_find_insertion_point(tree, item, &parent);
	if (*found)
	{
		/* exact match */
		return *found;
	}
	else if (!parent)
	{
		/* tree is empty so far */
		return NULL;
	}
	else if (tree->less_fn(parent, item, tree->less_fn_opaque))
	{
		/* parent is lower than item */
		return parent;
	}
	else
	{
		/* parent is greater than item, find the next lower entry */
		return genc_bt_prev_item(tree, parent);
	}
}

genc_bt_node_head_t* genc_bt_find_or_higher(genc_binary_tree_t* tree, genc_bt_node_head_t* item)
{
	genc_bt_node_head_t* parent = NULL;
	genc_bt_node_head_t** found = genc_bt_find_insertion_point(tree, item, &parent);
	if (*found)
	{
		/* exact match */
		return *found;
	}
	else if (!parent)
	{
		/* tree is empty so far */
		return NULL;
	}
	else if (tree->less_fn(parent, item, tree->less_fn_opaque))
	{
		/* parent is lower than item, find the next higher entry */
		return genc_bt_next_item(tree, parent);
	}
	else
	{
		/* parent is greater than item */
		return parent;
	}
}


void genc_bt_swap_trees(genc_binary_tree_t* tree_a, genc_binary_tree_t* tree_b)
{
	genc_binary_tree_t temp = *tree_a;
	*tree_a = *tree_b;
	*tree_b = temp;
}

genc_bool_t genc_bt_is_empty(genc_binary_tree_t* tree)
{
	return tree->root == NULL;
}
