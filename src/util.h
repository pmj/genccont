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

/* This file contains some definitions for homogenising across platforms and
 * some handy macros for dealing with embedded structs. All containers use it. */


#ifndef GENCCONT_UTIL_H
#define GENCCONT_UTIL_H

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

#if defined(__GNUC__)
#define GENC_UNUSED __attribute__((unused))
#else
#define GENC_UNUSED
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
	
	
/** genc_container_of(obj, cont_type, member_name)
 * Get pointer to struct object from a pointer to one of its members.
 * obj - pointer to a struct member, or NULL
 * cont_type - structure type of the containing object
 * member_name - name of the member of cont_type referenced by obj
 *
 * Similar to Linux's container_of macro, except it also handles NULL pointers
 * correctly, which is to say it evaluates to NULL when the obj is NULL. We also
 * try to support non-GCC compilers which don't support the ({ }) expression
 * syntax.
 *
 * Where the obj can be guaranteed to be non-NULL, genc_container_of_notnull()
 * may be used. It omits the NULL check and its behaviour is therefore undefined
 * if obj is indeed NULL.
 */ 
#ifndef genc_container_of
	
	/* function for avoiding multiple evaluation */
	static GENC_INLINE char* genc_container_of_helper(const void* obj, ptrdiff_t offset)
	{
		return (obj ? ((char*)obj - offset) : NULL);
	}
	/* function for avoiding multiple evaluation */
	static GENC_INLINE char* genc_container_of_notnull_helper(const void* obj, ptrdiff_t offset)
	{
		return ((char*)obj - offset);
	}
	
#ifdef __GNUC__
	
	/* the unused _p attribute is for causing a compiler warning if member_name of
	 * cont_type does not have same type as target of obj*/
#define genc_container_of(obj, cont_type, member_name) \
({ \
cont_type* _c = ((cont_type*)genc_container_of_helper((obj), offsetof(cont_type, member_name))); \
__typeof__(obj) __attribute__ ((unused)) _p = _c ? &_c->member_name : NULL; \
_c; \
})
	
#define genc_container_of_notnull(obj, cont_type, member_name) \
({ \
cont_type* _c = ((cont_type*)genc_container_of_notnull_helper((obj), offsetof(cont_type, member_name))); \
__typeof__(obj) __attribute__ ((unused)) _p = &_c->member_name; \
_c; \
})
	
	
#else
#define genc_container_of(obj, cont_type, member_name) \
((cont_type*)genc_container_of_helper((obj), offsetof(cont_type, member_name)))
#define genc_container_of_notnull(obj, cont_type, member_name) \
((cont_type*)genc_container_of_notnull_helper((obj), offsetof(cont_type, member_name)))
#endif
	
	static GENC_INLINE void* genc_member_of_helper(const void* obj, ptrdiff_t offset)
	{
		return obj ? ((char*)obj + offset) : NULL;
	}
	
#endif
	
#ifdef __cplusplus
} /* extern "C" */
#endif

#if defined(KERNEL) && defined(APPLE)
#undef ptrdiff_t
#endif

#endif
