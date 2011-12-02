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

void genc_dlist_init(struct dlist_head* head)
{
	head->next = head;
	head->prev = head;
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
	at->next = NULL;
	at->prev = NULL;
	return at;
}

int genc_dlist_is_empty(struct dlist_head* list)
{
	return list->next == list;
}

struct dlist_head* genc_dlist_remove_last(struct dlist_head* list)
{
	if (genc_dlist_is_empty(list))
		return NULL;
	return genc_dlist_remove(&list->prev);
}
