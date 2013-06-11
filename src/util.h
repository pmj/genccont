/*
 Copyright (c) 2011-2012 Phil Jordan <phil@philjordan.eu>/<phil@ssdcache.com>
 
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


/* We define a boolean type genc_bool_t to try to cover all standards cleanly
 * Use standard boolean type 'bool' for C99, C++ and GNU C. If not available,
 * use char; this assumes sizeof(bool) == sizeof(_Bool) == sizeof(char) === 1.
 *
 * We #define GENC_INLINE to be the target language's inline specifier, if it
 * has one. Old Cs don't have one, so this is only used together with the static storage
 * specifier to avoid linkage issues. */
#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || (defined(__GNUC__) && (!defined(__STRICT_ANSI__) || !__STRICT_ANSI__))
#define GENC_INLINE inline

#ifndef __cplusplus
#include <stdbool.h>
#endif
typedef bool genc_bool_t;

#elif (defined(__GNUC__) && defined(__STRICT_ANSI__) && __STRICT_ANSI__)
#define GENC_INLINE __inline__
typedef char genc_bool_t;

#else
#define GENC_INLINE
typedef char genc_bool_t;

#endif

#if defined(__GNUC__)
#define GENC_UNUSED __attribute__((unused))
#else
#define GENC_UNUSED
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

#ifndef __has_feature
  #define __has_feature(x) 0
#endif
#ifndef __has_extension
  #define __has_extension __has_feature
#endif


/* Where possible, make the helper functions const-correct via overloading.
 * C++ compilers obviously support it, but so does clang. For pure GCC,
 * we have a different solution. */
#if defined(__cplusplus) || __has_extension(attribute_overloadable)
#ifdef __cplusplus
#define GENC_OVERLOADABLE
#else
/* This is clang, basically */
#define GENC_OVERLOADABLE __attribute__((overloadable))
#endif
	/* function for avoiding multiple evaluation */
	static inline void* GENC_OVERLOADABLE genc_container_of_helper(void* obj, ptrdiff_t offset)
	{
		return (obj ? ((char*)obj - offset) : NULL);
	}
	static inline const void* GENC_OVERLOADABLE genc_container_of_helper(const void* obj, ptrdiff_t offset)
	{
		return (obj ? ((const char*)obj - offset) : NULL);
	}
	/* function for avoiding multiple evaluation */
	static inline const void* GENC_OVERLOADABLE genc_container_of_notnull_helper(const void* obj, ptrdiff_t offset)
	{
		return ((const char*)obj - offset);
	}
	static inline void* GENC_OVERLOADABLE genc_container_of_notnull_helper(void* obj, ptrdiff_t offset)
	{
		return ((char*)obj - offset);
	}

#else
	/* function for avoiding multiple evaluation */
	static GENC_INLINE void* genc_container_of_helper(void* obj, ptrdiff_t offset)
	{
		return (obj ? ((char*)obj - offset) : NULL);
	}
	/* function for avoiding multiple evaluation */
	static GENC_INLINE void* genc_container_of_notnull_helper(void* obj, ptrdiff_t offset)
	{
		return ((char*)obj - offset);
	}

#if __GNUC__
	/* GNU C has some builtin trickery that is almost as good as overloading */
	/* function for avoiding multiple evaluation */
	static GENC_INLINE const void* genc_container_of_const_helper(const void* obj, ptrdiff_t offset)
	{
		return (obj ? ((const char*)obj - offset) : NULL);
	}
	/* function for avoiding multiple evaluation */
	static GENC_INLINE const void* genc_container_of_const_notnull_helper(const void* obj, ptrdiff_t offset)
	{
		return ((const char*)obj - offset);
	}

#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__

#if defined(__cplusplus) || __has_extension(attribute_overloadable)
	/* the unused _p variable is for causing a compiler warning if member_name of
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

/* GCC builtin trickery (C mode only, no overloading possible) so that different helpers
 * get called for const and non-const types (avoids warnings even with -Wcast-qual).
 * Checks if pointers to const type and type are compatible - only the case if
 * type is already const - and dispatches to the 2 different helper functions. */
#define genc_container_of(obj, cont_type, member_name) \
({ \
cont_type* _c = \
	(cont_type*)(__builtin_choose_expr(__builtin_types_compatible_p(const cont_type*, cont_type*), \
		genc_container_of_const_helper, genc_container_of_helper) \
		((obj), offsetof(cont_type, member_name))); \
__typeof__(obj) __attribute__ ((unused)) _p = _c ? &_c->member_name : NULL; \
_c; \
})
	
#define genc_container_of_notnull(obj, cont_type, member_name) \
({ \
cont_type* _c = \
	(cont_type*)(__builtin_choose_expr(__builtin_types_compatible_p(const cont_type*, cont_type*), \
		genc_container_of_const_notnull_helper, genc_container_of_notnull_helper) \
			((obj), offsetof(cont_type, member_name))); \
__typeof__(obj) __attribute__ ((unused)) _p = &_c->member_name; \
_c; \
})

#endif
	
#else
#define genc_container_of(obj, cont_type, member_name) \
((cont_type*)genc_container_of_helper((obj), offsetof(cont_type, member_name)))
#define genc_container_of_notnull(obj, cont_type, member_name) \
((cont_type*)genc_container_of_notnull_helper((obj), offsetof(cont_type, member_name)))
#endif

#endif
	
#ifdef __cplusplus
} /* extern "C" */
#endif

#if defined(KERNEL) && defined(APPLE)
#undef ptrdiff_t
#endif

#ifdef __cplusplus
#define GENC_CXX_CAST(TYPE, EXPR) static_cast<TYPE>(EXPR)
#else
#define GENC_CXX_CAST(TYPE, EXPR) (EXPR)
#endif

#endif
