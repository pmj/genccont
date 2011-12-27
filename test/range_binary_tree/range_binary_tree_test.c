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

#include "../../src/range_binary_tree.h"

#include <stdio.h>
#include <assert.h>

int main(void)
{
	
	genc_range_binary_tree_item_t
		a = {{}, 10, 15 },
		b = {{}, 7, 9 },
		c = {{}, 4, 6 },
		d = {{}, 1, 2 },
		e = {{}, 3, 4 },
		f = {{}, 20, 25 },
		g = {{}, 35, 42 },
		h = {{}, 27, 29 },
		i = {{}, 17, 18 };
	
	genc_binary_tree_t tree;
	
	genc_range_binary_tree_init(&tree);
	
	int ok = genc_range_bt_insert(&tree, &a);
	assert(ok);
	ok = genc_range_bt_insert(&tree, &b);
	assert(ok);
	ok = genc_range_bt_insert(&tree, &c);
	assert(ok);
	ok = genc_range_bt_insert(&tree, &d);
	assert(ok);
	ok = genc_range_bt_insert(&tree, &e);
	assert(ok);
	ok = genc_range_bt_insert(&tree, &f);
	assert(ok);
	ok = genc_range_bt_insert(&tree, &g);
	assert(ok);
	ok = genc_range_bt_insert(&tree, &h);
	assert(ok);
	ok = genc_range_bt_insert(&tree, &i);
	assert(ok);
	
	genc_range_binary_tree_item_t test_range = {{}, 16, 19 };
	genc_range_bt_node_range_t overlap = genc_range_bt_find_overlap(&tree, &test_range);
	assert(overlap.start == &i);
	assert(overlap.end == &f);

	ok = genc_range_bt_insert(&tree, &test_range);
	assert(!ok);
	
	test_range.range_start = 11;
	test_range.range_end = 13;
	overlap = genc_range_bt_find_overlap(&tree, &test_range);
	assert(overlap.start == &a);
	assert(overlap.end == &i);
	
	test_range.range_start = 1;
	test_range.range_end = 4;
	overlap = genc_range_bt_find_overlap(&tree, &test_range);
	assert(overlap.start == &d);
	assert(overlap.end == &c);

	test_range.range_start = 2;
	test_range.range_end = 8;
	overlap = genc_range_bt_find_overlap(&tree, &test_range);
	assert(overlap.start == &e);
	assert(overlap.end == &a);

	test_range.range_start = 28;
	test_range.range_end = 30;
	overlap = genc_range_bt_find_overlap(&tree, &test_range);
	assert(overlap.start == &h);
	assert(overlap.end == &g);

	test_range.range_start = 17;
	test_range.range_end = 24;
	overlap = genc_range_bt_find_overlap(&tree, &test_range);
	assert(overlap.start == &i);
	assert(overlap.end == &h);

	test_range.range_start = 45;
	test_range.range_end = 47;
	overlap = genc_range_bt_find_overlap(&tree, &test_range);
	assert(overlap.start == NULL);
	assert(overlap.end == NULL);

	test_range.range_start = 28;
	test_range.range_end = 41;
	overlap = genc_range_bt_find_overlap(&tree, &test_range);
	assert(overlap.start == &h);
	assert(overlap.end == NULL);

	test_range.range_start = 25;
	test_range.range_end = 26;
	overlap = genc_range_bt_find_overlap(&tree, &test_range);
	assert(overlap.start == &h);
	assert(overlap.end == &h);

	test_range.range_start = 30;
	test_range.range_end = 33;
	overlap = genc_range_bt_find_overlap(&tree, &test_range);
	assert(overlap.start == &g);
	assert(overlap.end == &g);
	
	test_range.range_start = 11;
	test_range.range_end = 14;
	genc_range_binary_tree_item_t split = {};
	genc_range_bt_chop_result_t chop = genc_range_bt_chop_range(&tree, &test_range, &split);
	assert(a.range_start == 10);
	assert(a.range_end == 11);
	assert(split.range_start == 14);
	assert(split.range_end == 15);
	assert(chop.did_split);
	assert(chop.removed_node_list == NULL);
	assert(chop.start_truncated == &a);
	assert(chop.end_truncated_or_split == &split);

	test_range.range_start = 28;
	test_range.range_end = 40;
	chop = genc_range_bt_chop_range(&tree, &test_range, NULL);
	assert(h.range_start == 27);
	assert(h.range_end == 28);
	assert(g.range_start == 40);
	assert(g.range_end == 42);
	assert(!chop.did_split);
	assert(chop.removed_node_list == NULL);
	assert(chop.start_truncated == &h);
	assert(chop.end_truncated_or_split == &g);
	
	test_range.range_start = 1;
	test_range.range_end = 8;
	chop = genc_range_bt_chop_range(&tree, &test_range, NULL);
	assert(b.range_start == 8);
	assert(b.range_end == 9);
	assert(!chop.did_split);
	assert(chop.removed_node_list == &d.head);
	assert(chop.removed_node_list->right == &e.head);
	assert(chop.removed_node_list->right->right == &c.head);
	assert(chop.removed_node_list->right->right->right == NULL);
	assert(chop.start_truncated == NULL);
	assert(chop.end_truncated_or_split == &b);

	/* same again, nothing should happen this time */
	chop = genc_range_bt_chop_range(&tree, &test_range, NULL);
	assert(!chop.did_split);
	assert(chop.removed_node_list == NULL);
	assert(chop.start_truncated == NULL);
	assert(chop.end_truncated_or_split == NULL);

	test_range.range_start = 19;
	test_range.range_end = 26;
	chop = genc_range_bt_chop_range(&tree, &test_range, NULL);
	assert(!chop.did_split);
	assert(chop.removed_node_list == &f.head);
	assert(chop.removed_node_list->right == NULL);
	assert(chop.start_truncated == NULL);
	assert(chop.end_truncated_or_split == NULL);
	return 0;
}

