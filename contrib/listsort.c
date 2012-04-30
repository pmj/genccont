#include <stdio.h>

/*
 * Demonstration code for sorting a linked list.
 * 
 * The algorithm used is Mergesort, because that works really well
 * on linked lists, without requiring the O(N) extra space it needs
 * when you do it on arrays.
 * 
 * This code can handle singly and doubly linked lists, and
 * circular and linear lists too. For any serious application,
 * you'll probably want to remove the conditionals on `is_circular'
 * and `is_double' to adapt the code to your own purpose. 
 * 
 */

/*
 * This file is copyright 2001 Simon Tatham.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL SIMON TATHAM BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define FALSE 0
#define TRUE 1

typedef struct element element;
struct element {
    element *next, *prev;
    int i;
};

int cmp(element *a, element *b) {
    return a->i - b->i;
}

/*
 * This is the actual sort function. Notice that it returns the new
 * head of the list. (It has to, because the head will not
 * generally be the same element after the sort.) So unlike sorting
 * an array, where you can do
 * 
 *     sort(myarray);
 * 
 * you now have to do
 * 
 *     list = listsort(mylist);
 */
element *listsort(element *list, int is_circular, int is_double) {
    element *p, *q, *e, *tail, *oldhead;
    int insize, nmerges, psize, qsize, i;

    /*
     * Silly special case: if `list' was passed in as NULL, return
     * NULL immediately.
     */
    if (!list)
	return NULL;

    insize = 1;

    while (1) {
        p = list;
	oldhead = list;		       /* only used for circular linkage */
        list = NULL;
        tail = NULL;

        nmerges = 0;  /* count number of merges we do in this pass */

        while (p) {
            nmerges++;  /* there exists a merge to be done */
            /* step `insize' places along from p */
            q = p;
            psize = 0;
            for (i = 0; i < insize; i++) {
                psize++;
		if (is_circular)
		    q = (q->next == oldhead ? NULL : q->next);
		else
		    q = q->next;
                if (!q) break;
            }

            /* if q hasn't fallen off end, we have two lists to merge */
            qsize = insize;

            /* now we have two lists; merge them */
            while (psize > 0 || (qsize > 0 && q)) {

                /* decide whether next element of merge comes from p or q */
                if (psize == 0) {
		    /* p is empty; e must come from q. */
		    e = q; q = q->next; qsize--;
		    if (is_circular && q == oldhead) q = NULL;
		} else if (qsize == 0 || !q) {
		    /* q is empty; e must come from p. */
		    e = p; p = p->next; psize--;
		    if (is_circular && p == oldhead) p = NULL;
		} else if (cmp(p,q) <= 0) {
		    /* First element of p is lower (or same);
		     * e must come from p. */
		    e = p; p = p->next; psize--;
		    if (is_circular && p == oldhead) p = NULL;
		} else {
		    /* First element of q is lower; e must come from q. */
		    e = q; q = q->next; qsize--;
		    if (is_circular && q == oldhead) q = NULL;
		}

                /* add the next element to the merged list */
		if (tail) {
		    tail->next = e;
		} else {
		    list = e;
		}
		if (is_double) {
		    /* Maintain reverse pointers in a doubly linked list. */
		    e->prev = tail;
		}
		tail = e;
            }

            /* now p has stepped `insize' places along, and q has too */
            p = q;
        }
	if (is_circular) {
	    tail->next = list;
	    if (is_double)
		list->prev = tail;
	} else
	    tail->next = NULL;

        /* If we have done only one merge, we're finished. */
        if (nmerges <= 1)   /* allow for nmerges==0, the empty list case */
            return list;

        /* Otherwise repeat, merging lists twice the size */
        insize *= 2;
    }
}

/*
 * Small test rig with three test orders. The list length 13 is
 * chosen because that means some passes will have an extra list at
 * the end and some will not.
 */

int main(void) {
    #define n 13
    element k[n], *head, *p;
    int is_circular, is_double;

    int order[][n] = {
        { 0,1,2,3,4,5,6,7,8,9,10,11,12 },
        { 6,2,8,4,11,1,12,7,3,9,5,0,10 },
        { 12,11,10,9,8,7,6,5,4,3,2,1,0 },
    };
    int i, j;

    for (j = 0; j < n; j++)
        k[j].i = j;

    listsort(NULL, 0, 0);

    for (is_circular = 0; is_circular < 2; is_circular++) {
	for (is_double = 0; is_double < 2; is_double++) {
	    for (i = 0; i < sizeof(order)/sizeof(*order); i++) {
		int *ord = order[i];
		head = &k[ord[0]];
		for (j = 0; j < n; j++) {
		    if (j == n-1)
			k[ord[j]].next = (is_circular ? &k[ord[0]] :
					  NULL);
		    else
			k[ord[j]].next = &k[ord[j+1]];
		    if (is_double) {
			if (j == 0)
			    k[ord[j]].prev = (is_circular ? &k[ord[n-1]] :
					      NULL);
			else
			    k[ord[j]].prev = &k[ord[j-1]];
		    }
		}

		printf("before:");
		p = head;
		do {
		    printf(" %d", p->i);
		    if (is_double) {
			if (p->next && p->next->prev != p)
			    printf(" [REVERSE LINK ERROR!]");
		    }
		    p = p->next;
		} while (is_circular ? (p != head) : (p != NULL));
		printf("\n");
		head = listsort(head, is_circular, is_double);
		printf(" after:");
		p = head;
		do {
		    printf(" %d", p->i);
		    if (is_double) {
			if (p->next && p->next->prev != p)
			    printf(" [REVERSE LINK ERROR!]");
		    }
		    p = p->next;
		} while (is_circular ? (p != head) : (p != NULL));
		printf("\n");
	    }
	}
    }
    return 0;
}
