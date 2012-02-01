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

#include "slist_queue.h"

void genc_slq_init(struct slist_queue* queue)
{
	queue->head = NULL;
	queue->tail = &queue->head;
}

/** Inserts the given item at the back of the queue. */
void genc_slq_push_back(struct slist_queue* queue, struct slist_head* new_item)
{
	queue->tail = genc_slist_insert_at(new_item, queue->tail);
}

/** Inserts the given at the front of the queue. */
void genc_slq_push_front(struct slist_queue* queue, struct slist_head* new_item)
{
	if (!queue->head)
	{
		/* adding to front of empty queue is identical to adding to its back (tail needs updating) */
		genc_slq_push_back(queue, new_item);
	}
	else
	{
		genc_slist_insert_at(new_item, &queue->head);
	}
}


/** Pops an item off the front of the queue, returning it. */
struct slist_head* genc_slq_pop_front(struct slist_queue* queue)
{
	struct slist_head* removed = genc_slist_remove_at(&queue->head);
	if (!removed) return NULL;
	if (!queue->head)
	{
		/* queue is now empty */
		queue->tail = &queue->head;
	}
	return removed;
}

genc_bool_t genc_slq_is_empty(struct slist_queue* queue)
{
	return !queue->head;
}

void genc_slq_swap(struct slist_queue* queue1, struct slist_queue* queue2)
{
	if (queue1 == queue2)
		return;
	
	struct slist_queue tmp_q = *queue1;
	*queue1 = *queue2;
	*queue2 = tmp_q;
		
	/* fix up empty queue tails if necessary*/
	if (queue1->tail == &queue2->head)
		queue1->tail = &queue1->head;
	if (queue2->tail == &queue1->head)
		queue2->tail = &queue2->head;
}

size_t genc_slq_length(slist_queue_t* queue)
{
	return genc_slist_length(queue->head);
}

struct slist_head* genc_slq_front(struct slist_queue* queue)
{
	return queue->head;
}

void genc_slq_splice_onto_end(slist_queue_t* onto_end_of_queue, slist_queue_t* from_queue)
{
	if (genc_slq_is_empty(from_queue))
		return;
	*onto_end_of_queue->tail = from_queue->head;
	onto_end_of_queue->tail = from_queue->tail;
	from_queue->head = NULL;
	from_queue->tail = &from_queue->head;
}
