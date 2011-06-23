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

/* for standard C typedefs, NULL, offsetof(), etc. */
#if defined(LINUX) && defined(__KERNEL__)
/* compiling the linux kernel (or rather a module) */
#include <linux/types.h>
#include <linux/stddef.h>
#elif defined(KERNEL) && defined(APPLE)
/* xnu kernel */
#include <IOKit/IOTypes.h>
/* xnu for some reason doesn't typedef ptrdiff_t. To avoid stepping on toes,
 * we'll temporarily re-#define it in case another header sets it */
#define ptrdiff_t __darwin_ptrdiff_t
#else
/* assume libc is available */
#include <stddef.h>
#endif


#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || (defined(__GNUC__) && (!defined(__STRICT_ANSI__) || !__STRICT_ANSI__))
#define GENC_INLINE inline
#elif (defined(__GNUC__) && defined(__STRICT_ANSI__) && __STRICT_ANSI__)
#define GENC_INLINE __inline__
#else
#define GENC_INLINE
#endif


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

/* Initialises an empty list. */
void genc_dlist_init(struct dlist_head* head);

/* predicate function type for filtering list entries, returns 0 for no match, non-0 for match */
typedef int (*genc_dlist_entry_pred_fn)(struct dlist_head* entry, void* data);

/** Locates a specific list entry based on the given predicate function.
 * Returns a pointer to the first matched element, or NULL if none is found.
 * Begins the search at the element following start_after; the last element tried
 * is the one before end_before. Thus, passing the list's head for both arguments
 * searches the entire list.
 */
struct dlist_head* genc_dlist_find_in_range(struct dlist_head* start_after, struct dlist_head* end_before, genc_dlist_entry_pred_fn pred, void* data);

/** Inserts a single new list element before the given list element.
 * Appending to the end of a list is done by inserting before the list's head.
 */
void genc_dlist_insert_before(struct dlist_head* new_entry, struct dlist_head* before);

/** Inserts a single new list element after an existing entry. Insert after the
 * list's head to insert at the beginning of the list.
 */
void genc_dlist_insert_after(struct dlist_head* new_entry, struct dlist_head* after);

/** Removes the list element at the given position.
 */
void genc_dlist_remove(struct dlist_head* at);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif

