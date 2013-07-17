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

#ifndef GENCCONT_SLIST_QUEUE_H
#define GENCCONT_SLIST_QUEUE_H

#include "slist.h"

#ifdef __cplusplus
extern "C" {
#endif

/** A simple list-based FIFO queue.
 * The front of the queue is the head of a singly linked list, the back of the queue is its tail.
 * For O(1) insertions at the back, a pointer to the tail is maintained.
 * Insertions at the front (or anywhere else in the queue) are also supported.
 */
struct slist_queue
{
	struct slist_head* head;
	/* Initialised to &head when empty and always updated to point at the 'next' member of the last list element. */
	struct slist_head** tail;
};
typedef struct slist_queue slist_queue_t;
typedef struct slist_queue genc_slist_queue_t;

/** Initialises an empty queue */
void genc_slq_init(struct slist_queue* queue);

/** Inserts the given item at the back of the queue. */
void genc_slq_push_back(struct slist_queue* queue, struct slist_head* new_item);

/** Pops an item off the front of the queue, returning it. NULL if queue is empty. */
struct slist_head* genc_slq_pop_front(struct slist_queue* queue);

/** Inserts the given at the front of the queue. */
void genc_slq_push_front(struct slist_queue* queue, struct slist_head* new_item);

/** Returns the item at the front of the queue, or NULL if the queue is empty.
 * Does NOT remove the returned item. */
struct slist_head* genc_slq_front(struct slist_queue* queue);

/* returns 0 if queue contains one or more items, 1 if empty */
genc_bool_t genc_slq_is_empty(struct slist_queue* queue);

/* Swaps the contents of the two queues. This isn't quite as trivial as swapping
 * the pointer values as the tail of an empty queue points to its head pointer,
 * which needs to be fixed up after swapping. */
void genc_slq_swap(struct slist_queue* queue1, struct slist_queue* queue2);

/* Length of the queue, O(N) runtime complexity */
size_t genc_slq_length(slist_queue_t* queue);

/* Moves all all elements of from_queue onto the end of onto_end_of_queue,
 * leaving from_queue empty. */
void genc_slq_splice_onto_end(slist_queue_t* onto_end_of_queue, slist_queue_t* from_queue);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** genc_slq_pop_front_object(queue, list_type, list_head_member_name)
 * Typed version of genc_slq_pop_front(). */
#define genc_slq_pop_front_object(queue, list_type, list_head_member_name) \
genc_container_of(genc_slq_pop_front(queue), list_type, list_head_member_name)

#define genc_slq_front_object(queue, list_type, list_head_member_name) \
genc_container_of(genc_slq_front(queue), list_type, list_head_member_name)

#endif
