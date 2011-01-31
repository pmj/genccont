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


struct slist_head** genc_slist_find_entry_ref(struct slist_head** start, genc_slist_entry_pred_fn pred)
{
	struct slist_head** cur = start;
	while (*cur)
	{
		if (pred(*cur))
		{
			return cur;
		}
		cur = &(*cur)->next;
	}
	return cur;
}

struct slist_head* genc_slist_find_entry(struct slist_head* start, genc_slist_entry_pred_fn pred)
{
	return *genc_slist_find_entry_ref(&start, pred);
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

