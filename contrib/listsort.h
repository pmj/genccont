//
//  listsort.h
//  ssdcache
//
//  Created by Phillip Jordan on 4/30/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ssdcache_listsort_h
#define ssdcache_listsort_h

#include "../src/slist.h"

typedef int (*genc_slist_element_compare_fn)(genc_slist_head_t* a, genc_slist_head_t* b);

#ifdef __cplusplus
extern "C" {
#endif

genc_slist_head_t* genc_slist_mergesort(genc_slist_head_t* list, genc_slist_element_compare_fn cmp);

#ifdef __cplusplus
}
#endif


#endif
