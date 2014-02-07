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

#ifndef GENCCONT_DLIST_H
#define GENCCONT_DLIST_H

#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

/* generic, circular doubly-linked list */

/* embed this in the list entry struct, and also use it as the list's head (initialised with genc_dlist_init()) */
struct dlist_head
{
	struct dlist_head* next;
	struct dlist_head* prev;
};
typedef struct dlist_head genc_dlist_head_t;

/* Initialises an empty list. */
void genc_dlist_init(struct dlist_head* head);

/* Sets the list membership head to all NULLs (not part of a list) */
void genc_dlist_head_zero(struct dlist_head* head);

/* Returns true if the list head is cleared to NULLs (not part of a list) */
bool genc_dlist_is_null(genc_dlist_head_t* head);

/* predicate function type for filtering list entries, returns 0 for no match, non-0 for match */
typedef genc_bool_t(*genc_dlist_entry_pred_fn)(struct dlist_head* entry, void* data);

/** Locates a specific list entry based on the given predicate function.
 * Returns a pointer to the first matched element, or NULL if none is found.
 * Begins the search at the element following start_after; the last element tried
 * is the one before end_before. Thus, passing the list's head for both arguments
 * searches the entire list.
 */
struct dlist_head* genc_dlist_find_in_range(struct dlist_head* start_after, struct dlist_head* end_before, genc_dlist_entry_pred_fn pred, void* data);

/** Locates a specific list entry based on the given predicate function.
 * Returns a pointer to the first matched element, or NULL if none is found.
 * Searches all items except the specific one passed as list, as this is assumed
 * to be the list anchoring dummy element.
 */
struct dlist_head* genc_dlist_find_in_list(struct dlist_head* list, genc_dlist_entry_pred_fn pred, void* data);

/** Inserts a single new list element before the given list element.
 * Appending to the end of a list is done by inserting before the list's head.
 */
void genc_dlist_insert_before(struct dlist_head* new_entry, struct dlist_head* before);

/** Inserts a single new list element after an existing entry. Insert after the
 * list's head to insert at the beginning of the list.
 */
void genc_dlist_insert_after(struct dlist_head* new_entry, struct dlist_head* after);

/** Removes the list element at the given position and returns it
 */
struct dlist_head* genc_dlist_remove(struct dlist_head* entry);

/** If the item's links are non-NULL, the item is removed from the list, and
 * true is returned. Otherwise, there is no list from which to remove it, so
 * false is returned. */
bool genc_dlist_remove_if_not_null(genc_dlist_head_t* item);

/** Returns 0 if the list contains elements, non-zero (1) if empty 
 */
int genc_dlist_is_empty(struct dlist_head* list);

/** Removes the list element at the end of the given list, or NULL if the list is empty
 */
struct dlist_head* genc_dlist_remove_last(struct dlist_head* list);

/** Removes the list element at the start of the given list, or NULL if the list is empty
 */
struct dlist_head* genc_dlist_remove_first(struct dlist_head* list);


/** Returns the last entry in the given list, or NULL if the list is empty */
struct dlist_head* genc_dlist_last(struct dlist_head* list);

/** Number of elements in the list, NOT including the head element. */
size_t genc_dlist_length(struct dlist_head* list);

/** genc_dlist_splice:
 * Remove a subrange of one list and insert it into another list, after the
 * specified item.
 *
 * For example, say we have these 2 lists:
 * 1: -> H1 <-> A <-> B <-> C <-
 * 2: -> H2 <-> D <-> E <-> F <-> G <-
 * 
 * And we call genc_dlist_splice(A, D, G), we end up with:
 *
 * 1: -> H1 <-> A <-> E <-> F <-> B <-> C <-
 * 2: -> H2 <-> D <-> G <-
 *
 * These semantics might seem odd, but they make it really easy to concatenate a
 * whole list. E.g. same initial condition as above, call:
 * genc_dlist_splice(H1, H2, H2)
 * and we end up with:
 * 1: -> H1 <-> D <-> E <-> F <-> G <-> A <-> B <-> C <-
 * 2: -> H2 <-
 */
void genc_dlist_splice(
	genc_dlist_head_t* into_after, genc_dlist_head_t* from_after, genc_dlist_head_t* from_before);

/* Similar to genc_dlist_splice but inserts into the destination list before the
 * specified element.
 * This is a helper to facilitate the following semantics (i.e. appending):
 *
 * 1: -> H1 <-> A <-> B <-> C <-
 * 2: -> H2 <-> D <-> E <-> F <-> G <-
 * genc_dlist_splice(H1, H2, H2)
 * 1: -> H1 <-> A <-> B <-> C <-> D <-> E <-> F <-> G <-
 * 2: -> H2 <-
 */
void genc_dlist_splice_before(
	genc_dlist_head_t* into_before, genc_dlist_head_t* from_after, genc_dlist_head_t* from_before);

void genc_assert_dlist_is_healthy(genc_dlist_head_t* list);

/** genc_dlist_remove_object(entry, list_type, list_head_member_name)
 * Typed version of genc_dlist_remove_at(). */
#define genc_dlist_remove_object(entry, list_type, list_head_member_name) \
genc_container_of(genc_dlist_remove(entry), list_type, list_head_member_name)

/* Iterate through a list with a for() loop, removing each element before entering the loop body.
 * genc_dlist_for_each(removed_element, list_head, list_type, list_head_member_name)
 * list_head - pointer to the list, must be of type struct dlist_head*.
 *   list_head itself won't be modified, but (*list_head)'s members will (necessarily).
 * removed_element - variable of pointer to list_type type, which references the
 *   element which has been removed from the list in the loop body.
 *   The loop body is responsible for reusing or freeing the memory.
 *
 * You may safely break out of this loop or use the 'continue' statement. When
 * using break, goto or return to leave the loop, the list remains in a
 * consistent state, with the elements not yet removed remaining in the list.
 */
#define genc_dlist_for_each_remove(removed_element, list_head, list_type, list_head_member_name) \
for ((removed_element = genc_dlist_is_empty(list_head) ? NULL : genc_dlist_remove_object((list_head)->next, list_type, list_head_member_name)); removed_element; (removed_element = genc_dlist_is_empty(list_head) ? NULL : genc_dlist_remove_object((list_head)->next, list_type, list_head_member_name)))

#define genc_dlist_remove_last_object(list, list_type, list_head_member_name) \
genc_container_of(genc_dlist_remove_last(list), list_type, list_head_member_name)

#define genc_dlist_remove_first_object(list, list_type, list_head_member_name) \
genc_container_of(genc_dlist_remove_first(list), list_type, list_head_member_name)

#define genc_dlist_last_object(list, list_type, list_head_member_name) \
genc_container_of(genc_dlist_last(list), list_type, list_head_member_name)

#define genc_dlist_insert_object_after(new_entry, after, list_head_member_name) \
genc_dlist_insert_after(&((new_entry)->list_head_member_name), &((after)->list_head_member_name))

#define genc_dlist_find_object_in_range(start_after, end_before, pred_fn, pred_data, list_type, list_head_member_name) \
genc_container_of( \
  genc_dlist_find_in_range(start_after, end_before, pred_fn, pred_data), list_type, list_head_member_name);

#define genc_dlist_find_object_in_list(list, pred_fn, pred_data, list_type, list_head_member_name) \
genc_container_of( \
  genc_dlist_find_in_list(list, pred_fn, pred_data), list_type, list_head_member_name);


/** Iterate over all elements in a list (except for the list head)
 * type - (struct/union) type of each object
 * member - member name of genc_dlist_head_t element in type. May be nested (e.g. foo[2].bar)
 * var - name of the loop variable (will be declared as type*, with for loop scope)
 * list_head - pointer to genc_dlist_head_t
 */
#define genc_dlist_for_each_object(type, member, var, list_head) \
for ( \
	type* var = genc_container_of_notnull(((list_head)->next), type, member); \
	&var->member != (list_head); \
	var = genc_container_of_notnull(var->member.next, type, member))


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif

