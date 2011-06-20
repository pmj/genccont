# GenCCont: Generic C Containers #

## Yet another container library: but why?

While working on a kernel-mode driver, I needed a simple, compact, singly-linked
list that had no dependencies I couldn't satisfy in kernel space. Surprisingly,
I found no such library. Despite my reluctance to succumb to NIH, I ended up
writing my own implementation. I surely can't be the only one needing such a thing,
so here it is. Later I also needed a queue and a generic hash table implementation,
which triggered development of the genc_chaining_hash_table.

## Design Goals

- **Minimal build requirements.** Most operating system kernels don't provide a
  full standard C library implementation. Many existing libraries rely on a
  standard C environment and fail to compile or link when extending a kernel.
  In some cases, you might want to compile with a C++ compiler instead of a C one.
	This is also supported, as is of course, including the headers from C++ when
	the library is built as true C - and vice versa.
- **A certain level of type safety.** C isn't known for its great support for
  generic programming or type safety, but in many cases, we can do better than
	casting everything to `void*`. In some cases, compiler extensions are required
	to do this, so some warnings will only appear on GCC and compatible compilers.
- **Minimise use of macros.** Some container implementations are built entirely as
  macros. I don't like macros, so the core functionality of the library is built
	as straight C functions. There are small macros for wrapping function calls to
	improve type safety of client code, but you don't have to use them.
- **Minimise dynamic memory allocations.** If you're writing non-trivial code in C,
  I assume you know what you're doing regarding memory allocations. This also
	ties into the minimal build requirements goal: `malloc()` and `free()` may
	not be available or desirable. Therefore, the list and queue assume no memory
	responsibility, whereas the hash table only allocates the table memory, not
	individual entries. The library doesn't do any fancy pointer value twiddling,
	so it won't interfere with conservative garbage collectors.
- **Permissive license.** I opted for the zlib license, which should be
  permissive enough for anyone, but if you'd prefer another open source license,
  get in touch. Free commercial use is permitted by the zlib license, but if it
	doesn't work in your specific case for some reason, we can work out commercial
	licensing.

## Getting Started

Add the .c files in src to your project's build system and it should compile out
of the box. `#include` the relevant headers from your code and off you go. The
chained hash table and the slist_queue depend on slist, but you can drop any
other files you don't need.

Let me
know (ideally via a github pull request!) if it didn't build out of the box for
your environment and I'll change the code. The build is currently tested as
working for normal
UNIX userspace and compiling Mac OS X kernel extensions. I'm aiming to support
even exotic environments, as long as they have a fairly standards-compliant C
or C++ compiler and linker.

## Documentation

### Singly linked list

The src/slist.h header file is very well documented, but to get you started:

Add a `struct slist_head` element to your list element (or create a new struct
that contains both a `slist_head` and your list element as a pointer or value).
Allocating and freeing instances of this struct is up to you. Membership in
multiple lists (or multiple membership in the same list) is possible by simply
adding more `slist_head` members (or even an array of them) to your list entry
struct.

A list is simply a pointer to a `struct slist_head`, initalised to NULL. I will
refer to this as the list's head pointer.

#### Insertion

List insertion is done with either `genc_slist_insert_at()` or `genc_slist_insert_after()`, and is O(1).

The former takes a pointer to an element's 'next' pointer or the list's head
pointer (`struct slist_head**`) as its position argument (the second argument).

This lets allows insertion at the beginning of a list and insertion before an
arbitrary element without re-traversing the whole list (we can't
step *back* in a singly linked list). `genc_slist_insert_after()` only needs a
pointer to the element *before* the insertion point, as it will modify that
element's `next` link.

The first argument is a pointer to the `slist_head` member of your list entry
struct instance that you want to insert.

To insert at the *end* of a list, find the tail pointer by calling
`genc_slist_find_tail()` on the list and using the result as the positional
argument to `genc_slist_insert_at()`. This is O(N) of course, so if you're doing
this a lot, consider using the `slist_queue`, which has an O(1) operation for this.

The insertion functions only ever insert one element, and inserting an element
that already uses the given `slist_head` for membership in another list will
likely cause bugs. Remove from the old list first, then insert into the new one.

To splice a whole list into another, use `genc_slist_splice(into, from)`. This has O(N)
time complexity, where N is the number of elements in the source (`from`) list.

#### Removal

Similarly to insertion, removal is possible either at a specific position via a
link pointer reference with `genc_slist_remove_at` or *after* a specific entry
with `genc_slist_remove_after()`. Both have O(1) time complexity and return the removed
element or NULL if there was no element to remove. Freeing the memory
is up to you.

There is a typed macro version of the `genc_slist_remove_at` function:
`genc_slist_remove_object_at(at, list_type, list_head_member_name)`. This assumes
the list contains elements of type `list_type` which were inserted using the (possibly
nested) list head member named `list_head_member_name`. The return type is therefore
`list_type*`.

There is a bulk removal loop macro, `genc_slist_for_each_remove()` which acts
like a for loop over each removed list element. Look at its definition in the
header for details and an example.

#### Search and iteration

Iterating through list elements one at a time is possible via the macro `genc_slist_next()`, which is type-aware. 

Linear search is possible via the `genc_slist_find_entry()` and `genc_slist_find_entry_ref()`
functions, which take a start position (as a list item pointer or list pointer
reference, respectively), a predicate function pointer matching
type `genc_slist_entry_pred_fn` and an opaque data pointer which will be passed
to each call of the predicate. The first matching entry or the pointer to it are
returned, respectively. If none match, a NULL pointer or the tail pointer are
returned, respectively. A typed version is available as the macro `genc_slist_find_obj()`.

In addition to the bulk removal loop mentioned above, the `genc_slist_for_each_ref()`
and `genc_slist_for_each()` allow you to conveniently looping through a list of
elements with a specified type. `genc_slist_for_each_head_ref()` is similar, but
operates directly on `slist_head` pointers. Check the definitions in the header
for details and examples.

## Plans/TODO

- Documentation for the `slist_queue` and the chained hash table.
- Templated C++ wrappers where appropriate.
- More data structures: e.g. doubly linked list and queue; hash tables with other
memory layout and collision resolution strategies; trees; etc.
- Wider testing (and support) of different platforms and compilers.
