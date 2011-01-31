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

#include "../../src/slist.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

struct A
{
	int x;
	int y;
};

struct B
{
	int z;
	struct A a;
};


struct int_list
{
	int val;
	struct slist_head head;
};

struct slist_head* setup_test_list()
{
	struct slist_head* head = NULL;
	int i;
	for (i = 0; i < 10; ++i)
	{
		struct int_list* e = calloc(1, sizeof(struct int_list));
		e->val = i;
		genc_slist_insert_at(&e->head, &head);
	}
	return head;
}

void free_test_list(struct slist_head* l)
{
	struct int_list* d = NULL;
	genc_slist_for_each_remove(d, &l, struct int_list, head)
	{
		free(d);
	}
}

int main()
{
	// check container_of
	struct B b;
	struct A* pa = &b.a;
	struct int_list* cur = NULL;
	
	assert(genc_container_of(pa, struct B, a) == &b);

	const struct B cb = {};
	const struct A* pca = &cb.a;
	assert(genc_container_of(pca, const struct B, a) == &cb);
	
	struct slist_head* list = setup_test_list();
	
	// iteration
	genc_slist_for_each(cur, list, struct int_list, head)
	{
		printf("%p: %d\n", cur, cur->val);
	}
	
	free_test_list(list);
	
	return 0;
}
