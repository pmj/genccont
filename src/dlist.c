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

#include "dlist.h"
#ifndef KERNEL
#include <assert.h>
#endif

void genc_dlist_init(struct dlist_head* head)
{
	head->next = head;
	head->prev = head;
}

void genc_dlist_head_zero(struct dlist_head* head)
{
	head->next = head->prev = NULL;
}

bool genc_dlist_is_null(genc_dlist_head_t* head)
{
	bool is_null = (head->next == NULL);
	assert(is_null == (head->prev == NULL));
	return is_null;
}

struct dlist_head* genc_dlist_find_in_range(struct dlist_head* start_after, struct dlist_head* end_before, genc_dlist_entry_pred_fn pred, void* data)
{
	struct dlist_head* cur = start_after->next;
	for (; cur != end_before; cur = cur->next)
	{
		if (pred(cur, data))
			return cur;
	}
	return NULL;
}

/** Inserts a single new list element before the given list element.
 * Appending to the end of a list is done by inserting before the list's head.
 */
void genc_dlist_insert_before(struct dlist_head* new_entry, struct dlist_head* before)
{
	new_entry->next = before;
	new_entry->prev = before->prev;
	before->prev->next = new_entry;
	before->prev = new_entry;
}

/** Inserts a single new list element after an existing entry. Insert after the
 * list's head to insert at the beginning of the list.
 */
void genc_dlist_insert_after(struct dlist_head* new_entry, struct dlist_head* after)
{
	new_entry->prev = after;
	new_entry->next = after->next;
	after->next->prev = new_entry;
	after->next = new_entry;
}

/** Removes the list element at the given position.
 */
struct dlist_head* genc_dlist_remove(struct dlist_head* at)
{
	at->prev->next = at->next;
	at->next->prev = at->prev;
	genc_dlist_head_zero(at);
	return at;
}

bool genc_dlist_remove_if_not_null(genc_dlist_head_t* item)
{
	if (genc_dlist_is_null(item))
		return false;
	genc_dlist_remove(item);
	return true;
}

int genc_dlist_is_empty(struct dlist_head* list)
{
	return list->next == list;
}

struct dlist_head* genc_dlist_last(struct dlist_head* list)
{
	if (genc_dlist_is_empty(list))
		return NULL;
	return list->prev;
}

struct dlist_head* genc_dlist_remove_last(struct dlist_head* list)
{
	if (genc_dlist_is_empty(list))
		return NULL;
	return genc_dlist_remove(list->prev);
}

struct dlist_head* genc_dlist_remove_first(struct dlist_head* list)
{
	if (genc_dlist_is_empty(list))
		return NULL;
	return genc_dlist_remove(list->next);
}

size_t genc_dlist_length(struct dlist_head* list)
{
	struct dlist_head* cur = list->next;
	size_t length = 0;
	while (cur != list)
	{
		++length;
		cur = cur->next;
	}
	return length;
}

struct dlist_head* genc_dlist_find_in_list(struct dlist_head* list, genc_dlist_entry_pred_fn pred, void* data)
{
	return genc_dlist_find_in_range(list, list, pred, data);
}

struct genc_dlist_range
{
	genc_dlist_head_t* first;
	genc_dlist_head_t* last;
};
typedef struct genc_dlist_range genc_dlist_range_t;

static genc_dlist_range_t genc_dlist_remove_range(genc_dlist_head_t* after, genc_dlist_head_t* before)
{
	genc_dlist_head_t* first = after->next;
	genc_dlist_head_t* last = before->prev;
	genc_dlist_range_t removed = { NULL, NULL };
	if (first == before)
	{
		// no-op, trying to splice a zero-length sublist
		assert(last == after);
		return removed;
	}
	
	after->next = before;
	before->prev = after;
	
	first->prev = NULL;
	last->next = NULL;
	
	removed.first = first;
	removed.last = last;
	return removed;
}

static void genc_dlist_insert_range_after(genc_dlist_range_t range, struct dlist_head* after)
{
	range.first->prev = after;
	range.last->next = after->next;
	after->next->prev = range.last;
	after->next = range.first;
}


void genc_dlist_splice(
	genc_dlist_head_t* into_after, genc_dlist_head_t* from_after, genc_dlist_head_t* from_before)
{
	genc_dlist_range_t range = genc_dlist_remove_range(from_after, from_before);
	if (range.first == NULL)
		return;
	
	genc_dlist_insert_range_after(range, into_after);
}

static void genc_dlist_insert_range_before(genc_dlist_range_t range, struct dlist_head* before)
{
	range.last->next = before;
	range.first->prev = before->prev;
	before->prev->next = range.first;
	before->prev = range.last;
}

void genc_dlist_splice_before(
	genc_dlist_head_t* into_before, genc_dlist_head_t* from_after, genc_dlist_head_t* from_before)
{
	genc_dlist_range_t range = genc_dlist_remove_range(from_after, from_before);
	if (range.first == NULL)
		return;
	
	genc_dlist_insert_range_before(range, into_before);
}

void genc_assert_dlist_is_healthy(genc_dlist_head_t* list)
{
	assert(list->next != NULL);
	assert(list->prev != NULL);
	genc_dlist_head_t* prev = list;
	genc_dlist_head_t* cur = list->next;
	genc_dlist_head_t* lagging = cur;
	while (cur != list)
	{
		assert(cur->next != NULL);
		assert(cur->prev != NULL);
		assert(cur->prev == prev);
		prev = cur;
		cur = cur->next;
		if (cur == list)
			break;
		
		assert(cur != lagging);
		assert(cur->next != NULL);
		assert(cur->prev != NULL);
		assert(cur->prev == prev);
		prev = cur;
		cur = cur->next;
		lagging = lagging->next;
		assert(cur != lagging);
	}
}

