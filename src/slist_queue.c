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

int genc_slq_is_empty(struct slist_queue* queue)
{
	return !queue->head;
}
