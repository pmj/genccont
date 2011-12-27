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

#include "../../src/binary_tree.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

struct btt_item
{
	genc_bt_node_head_t bt_head;
	
	int key;
	int data;
};
typedef struct btt_item btt_item_t;

static int dummy;

static int btt_item_less(genc_bt_node_head_t* a, genc_bt_node_head_t* b, void* opaque)
{
	assert(opaque == &dummy);
	btt_item_t* item_a = genc_container_of_notnull(a, btt_item_t, bt_head);
	btt_item_t* item_b = genc_container_of_notnull(b, btt_item_t, bt_head);
	return item_a->key < item_b->key;
}

static void check_order_invariant(genc_binary_tree_t* tree, int num_expected, int do_print)
{
	int i = 0;
	int prev_key = 0;
	btt_item_t* cur = genc_bt_first_obj(tree, btt_item_t, bt_head);
	while (cur)
	{
		++i;
		assert(cur->key == -cur->data);
		assert(cur->key > 0);
		if (do_print) printf("%i\n", cur->key);
		if (prev_key != 0)
		{
			assert(cur->key > prev_key);
		}
		prev_key = cur->key;
		cur = genc_bt_next_obj(tree, cur, btt_item_t, bt_head);
	}
	assert(i == num_expected);
	if (do_print) printf("\n");
	
	i = 0;
	prev_key = 0;
	cur = genc_bt_last_obj(tree, btt_item_t, bt_head);
	while (cur)
	{
		++i;
		assert(cur->key == -cur->data);
		assert(cur->key > 0);
		if (do_print) printf("%i\n", cur->key);
		if (prev_key != 0)
		{
			assert(cur->key < prev_key);
		}
		prev_key = cur->key;
		cur = genc_bt_prev_obj(tree, cur, btt_item_t, bt_head);
	}
	assert(i == num_expected);
	if (do_print) printf("\n");
}


void test_manual()
{
	genc_binary_tree_t* tree = malloc(sizeof(genc_binary_tree_t));
	genc_binary_tree_init(tree, btt_item_less, &dummy);
	
	assert(genc_bt_first_obj(tree, btt_item_t, bt_head) == NULL);
	assert(genc_bt_last_obj(tree, btt_item_t, bt_head) == NULL);
	
	btt_item_t a = { {}, 10, -10 };
	btt_item_t b = { {}, 4, -4 };
	btt_item_t c = { {}, 15, -15 };
	btt_item_t d = { {}, 2, -2 };
	btt_item_t e = { {}, 8, -8 };
	btt_item_t f = { {}, 9, -9 };
	btt_item_t g = { {}, 6, -6 };
	btt_item_t h = { {}, 7, -7 };
	
	btt_item_t f2 = { {}, 9, -9 };

	check_order_invariant(tree, 0, 0);

	int ok = genc_bt_insert(tree, &a.bt_head);
	assert(ok);
	check_order_invariant(tree, 1, 0);
	ok = genc_bt_insert(tree, &b.bt_head);
	assert(ok);
	check_order_invariant(tree, 2, 0);
	ok = genc_bt_insert(tree, &c.bt_head);
	assert(ok);
	check_order_invariant(tree, 3, 0);
	ok = genc_bt_insert(tree, &d.bt_head);
	assert(ok);
	check_order_invariant(tree, 4, 0);
	ok = genc_bt_insert(tree, &e.bt_head);
	assert(ok);
	check_order_invariant(tree, 5, 0);
	ok = genc_bt_insert(tree, &f.bt_head);
	assert(ok);
	check_order_invariant(tree, 6, 0);
	ok = genc_bt_insert(tree, &g.bt_head);
	assert(ok);
	check_order_invariant(tree, 7, 0);
	ok = genc_bt_insert(tree, &h.bt_head);
	assert(ok);
	check_order_invariant(tree, 8, 0);
	
	{
		btt_item_t find_item = { {} };
		find_item.key = 3;
		genc_bt_node_head_t* found = genc_bt_find_or_lower(tree, &find_item.bt_head);
		assert(found == &d.bt_head);

		find_item.key = 2;
		found = genc_bt_find_or_lower(tree, &find_item.bt_head);
		assert(found == &d.bt_head);

		find_item.key = 6;
		found = genc_bt_find_or_lower(tree, &find_item.bt_head);
		assert(found == &g.bt_head);

		find_item.key = 1;
		found = genc_bt_find_or_lower(tree, &find_item.bt_head);
		assert(found == NULL);

		find_item.key = 15;
		found = genc_bt_find_or_lower(tree, &find_item.bt_head);
		assert(found == &c.bt_head);

		find_item.key = 17;
		found = genc_bt_find_or_lower(tree, &find_item.bt_head);
		assert(found == &c.bt_head);

		find_item.key = 14;
		found = genc_bt_find_or_lower(tree, &find_item.bt_head);
		assert(found == &a.bt_head);

		find_item.key = 5;
		found = genc_bt_find_or_lower(tree, &find_item.bt_head);
		assert(found == &b.bt_head);

	}
	
	/* try to insert duplicate, this should fail */
	ok = genc_bt_insert(tree, &f2.bt_head);
	assert(!ok);
	check_order_invariant(tree, 8, 0);
	
	genc_bt_remove(tree, &h.bt_head);
	check_order_invariant(tree, 7, 0);
	
	ok = genc_bt_insert(tree, &h.bt_head);
	assert(ok);
	check_order_invariant(tree, 8, 0);
	
	genc_bt_remove(tree, &b.bt_head);
	check_order_invariant(tree, 7, 0);
	
	genc_bt_remove(tree, &a.bt_head);
	check_order_invariant(tree, 6, 0);
	
	genc_bt_remove(tree, &g.bt_head);
	check_order_invariant(tree, 5, 0);
	
	genc_bt_remove(tree, &e.bt_head);
	check_order_invariant(tree, 4, 0);
	
	genc_bt_remove(tree, &c.bt_head);
	check_order_invariant(tree, 3, 0);
	
	genc_bt_remove(tree, &f.bt_head);
	check_order_invariant(tree, 2, 0);
	
	genc_bt_remove(tree, &d.bt_head);
	check_order_invariant(tree, 1, 0);
	
	genc_bt_remove(tree, &h.bt_head);
	check_order_invariant(tree, 0, 0);
	
	free(tree);
}

// do lots of pseudo-random insertions and removals, checking consistency at every step
void test_random()
{
	srand(42);
	int i, j;
	for (i = 0; i < 20; ++i)
	{
		size_t num_items = 10 + rand() % 1000;
		genc_binary_tree_t* tree = malloc(sizeof(genc_binary_tree_t));
		genc_binary_tree_init(tree, btt_item_less, &dummy);

		printf("Testing with %lu items\n", num_items);
		btt_item_t* items = calloc(sizeof(btt_item_t), num_items);
		
		for (j = 0; j < num_items; ++j)
		{
			items[j].key = j + 1;
			items[j].data = 1;
		}
		
		int inserted = 0;
		
		for (j = 0; j < num_items * 10; ++j)
		{
			size_t item = rand() % num_items;
			if (items[item].data > 0)
			{
				items[item].data = -items[item].key;
				int ok = genc_bt_insert(tree, &items[item].bt_head);
				if (!ok)
				{
					printf("failed to insert item %lu on iteration %i\n", item, j);
					assert(items[item].key == item + 1);
				}
				assert(ok);
				++inserted;
				check_order_invariant(tree, inserted, 0);
			}
			else
			{
				btt_item_t find = {{}, item + 1, 1};
				btt_item_t* found = genc_bt_find_obj(tree, &find, btt_item_t, bt_head);
				assert(found == items + item);
				assert(found->key == -found->data);
				genc_bt_remove(tree, &items[item].bt_head);
				items[item].data = 1;
				--inserted;
				check_order_invariant(tree, inserted, 0);
			}
		}
		
		while (inserted > num_items / 30)
		{
			size_t item = rand() % num_items;
			if (items[item].data < 0)
			{
				genc_bt_remove(tree, &items[item].bt_head);
				items[item].data = 1;
				--inserted;
				check_order_invariant(tree, inserted, 0);
			}
		}
		if (inserted > 0)
			for (j = 0; j < num_items; ++j)
			{
				if (items[j].data < 0)
				{
					genc_bt_remove(tree, &items[j].bt_head);
					items[j].data = 1;
					--inserted;
					check_order_invariant(tree, inserted, 0);
				}
			}
		assert(inserted == 0);
		free(items);
		free(tree);
	}
}

int main()
{
	test_manual();
	test_random();
	return 0;
}
