# GenCCont: Generic C Containers #

While working on a kernel-mode driver, I needed a simple, compact, singly-linked
list that had no dependencies I couldn't satisfy in kernel space. Surprisingly,
I found no such library. Despite my reluctance to succumb to NIH, I ended up
writing my own implementation. I surely can't be the only one needing such a thing,
so here it is. I have vague plans to extend this beyond linked lists, hence
"containers".

The src/slist.h header file is documented, just #include it in your source and link
against src/slist.c, and you're in business. It will work with C++ as well, so you
can use it in C/C++ interface code. I'm planning to add some C++ syntactic sugar at
some point.

