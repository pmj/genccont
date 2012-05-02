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


#include "slist.h"


struct slist_head** genc_slist_find_entry_ref(struct slist_head** start, genc_slist_entry_pred_fn pred, void* data)
{
	struct slist_head** cur = start;
	while (*cur)
	{
		if (pred(*cur, data))
		{
			return cur;
		}
		cur = &(*cur)->next;
	}
	return cur;
}

struct slist_head* genc_slist_find_entry(struct slist_head* start, genc_slist_entry_pred_fn pred, void* data)
{
	return *genc_slist_find_entry_ref(&start, pred, data);
}

struct slist_head** genc_slist_insert_at(struct slist_head* new_entry, struct slist_head** at)
{
	new_entry->next = *at;
	*at = new_entry;
	return &new_entry->next;
}

void genc_slist_insert_after(struct slist_head* new_entry, struct slist_head* after_entry)
{
	genc_slist_insert_at(new_entry, &after_entry->next);
}


struct slist_head* genc_slist_remove_at(struct slist_head** at)
{
	struct slist_head* el = *at;
	if (!el) return NULL;
	*at = el->next;
	el->next = NULL;
	return el;
}

/** Removes the element following the given list element and returns it, or NULL if there was no such element.
 */
struct slist_head* genc_slist_remove_after(struct slist_head* after_entry)
{
	if (!after_entry) return NULL;
	return genc_slist_remove_at(&after_entry->next);
}

struct slist_head** genc_slist_find_tail(struct slist_head** head)
{
	struct slist_head** cur = head;
	while (*cur)
	{
		cur = &(*cur)->next;
	}
	return cur;
}

struct slist_head** genc_slist_splice(struct slist_head** into, struct slist_head** from)
{
	struct slist_head** from_tail;
	if (!*from)
		return into;
	
	from_tail = genc_slist_find_tail(from);
	*from_tail = *into;
	*into = *from;
	*from = NULL;
	return from_tail;
}

size_t genc_slist_length(struct slist_head* list)
{
	size_t len = 0;
	while (list)
	{
		++len;
		list = list->next;
	}
	return len;
}

genc_bool_t genc_slist_is_empty(genc_slist_head_t* list)
{
	return list == NULL;
}

genc_slist_head_t** genc_slist_find_ref(genc_slist_head_t* item, genc_slist_head_t** list)
{
	while (*list)
	{
		if (*list == item)
			return list;
		list = &(*list)->next;
	}
	return list;
}

void genc_slist_stack_init(genc_slist_stack_with_size_t* stack)
{
	stack->head = NULL;
	stack->size = 0;
}

struct slist_head* genc_slist_stack_pop(genc_slist_stack_with_size_t* stack)
{
	genc_slist_head_t* item = genc_slist_remove_at(&stack->head);
	if (item)
		--stack->size;
	return item;
}

void genc_slist_stack_push(genc_slist_stack_with_size_t* stack, struct slist_head* item)
{
	genc_slist_insert_at(item, &stack->head);
	++stack->size;
}

/* Given two finite lists with presumed common substructure, find that common tail.
 * Runtime complexity: O(n)
 * Returns the tail ref of list_a if no shared list elements. */
struct genc_slist_ref_pair genc_slist_find_common_tail_refs(struct slist_head** list_a, struct slist_head** list_b)
{
	size_t len_a = genc_slist_length(*list_a);
	size_t len_b = genc_slist_length(*list_b);
	
	/* Drop items off the front of one of the lists until they are the same length */
	if (len_a > len_b)
	{
		while (len_a > len_b)
		{
			list_a = &(*list_a)->next;
			--len_a;
		}
	}
	else
	{
		while (len_b > len_a)
		{
			list_b = &(*list_b)->next;
			--len_b;
		}
	}
	
	/* Locate the first common node */
	while (*list_a && (*list_a) != (*list_b))
	{
		list_a = &(*list_a)->next;
		list_b = &(*list_b)->next;
	}
	
	struct genc_slist_ref_pair pair = {{ list_a, list_b }};
	return pair;
}

/* Given two finite lists with presumed common substructure, find that common tail.
 * Runtime complexity: O(n); returns NULL if no common tail */
struct slist_head* genc_slist_find_common_tail(struct slist_head* list_a, struct slist_head* list_b)
{
	return *(genc_slist_find_common_tail_refs(&list_a, &list_b).refs[0]);
}
