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

#include "../../src/slist_queue.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


struct int_list
{
	int val;
	struct slist_head head;
};

struct int_list* new_int_list(int val)
{
	struct int_list* item = (struct int_list*)calloc(1, sizeof(struct int_list));
	item->val = val;
	return item;
}

int main()
{
	struct int_list* item = NULL;
	struct slist_queue queue;
	genc_slq_init(&queue);
	
	assert(queue.head == NULL);
	assert(queue.tail == &queue.head);
	
	genc_slq_push_back(&queue, &new_int_list(5)->head);
	item = genc_slq_pop_front_object(&queue, struct int_list, head);
	assert(item != NULL);
	assert(item->val == 5);
	assert(queue.head == NULL);
	assert(queue.tail == &queue.head);
	free(item);
	
	genc_slq_push_front(&queue, &new_int_list(5)->head);
	item = genc_slq_pop_front_object(&queue, struct int_list, head);
	assert(item != NULL);
	assert(item->val == 5);
	assert(queue.head == NULL);
	assert(queue.tail == &queue.head);
	free(item);

	genc_slq_push_back(&queue, &new_int_list(1)->head);
	genc_slq_push_back(&queue, &new_int_list(2)->head);
	genc_slq_push_back(&queue, &new_int_list(3)->head);
	genc_slq_push_front(&queue, &new_int_list(0)->head);
	genc_slq_push_back(&queue, &new_int_list(4)->head);
	
	slist_queue_t other_queue;
	genc_slq_init(&other_queue);
	
	genc_slq_push_back(&other_queue, &new_int_list(6)->head);
	genc_slq_push_back(&other_queue, &new_int_list(7)->head);
	genc_slq_push_front(&other_queue, &new_int_list(5)->head);
	genc_slq_push_back(&other_queue, &new_int_list(8)->head);
	
	assert(genc_slq_length(&queue) == 5);
	assert(genc_slq_length(&other_queue) == 4);
	
	genc_slq_splice_onto_end(&queue, &other_queue);

	assert(genc_slq_length(&queue) == 9);
	assert(genc_slq_length(&other_queue) == 0);
	
	assert(genc_slq_is_empty(&other_queue));
	
	item = genc_slq_pop_front_object(&queue, struct int_list, head);
	assert(item->val == 0);
	free(item);

	item = genc_slq_pop_front_object(&queue, struct int_list, head);
	assert(item->val == 1);
	free(item);

	item = genc_slq_pop_front_object(&queue, struct int_list, head);
	assert(item->val == 2);
	free(item);
	
	item = genc_slq_pop_front_object(&queue, struct int_list, head);
	assert(item->val == 3);
	free(item);
	
	item = genc_slq_pop_front_object(&queue, struct int_list, head);
	assert(item->val == 4);
	free(item);

	item = genc_slq_pop_front_object(&queue, struct int_list, head);
	assert(item->val == 5);
	free(item);

	item = genc_slq_pop_front_object(&queue, struct int_list, head);
	assert(item->val == 6);
	free(item);

	item = genc_slq_pop_front_object(&queue, struct int_list, head);
	assert(item->val == 7);
	free(item);

	item = genc_slq_pop_front_object(&queue, struct int_list, head);
	assert(item->val == 8);
	free(item);


	assert(queue.head == NULL);
	assert(queue.tail == &queue.head);

	assert(genc_slq_is_empty(&queue));

	return 0;
}
