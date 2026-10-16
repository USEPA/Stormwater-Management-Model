/*
 *   fill_malloc.c
 *
 *   Lets a test simulate "uninitialised memory": while fill_malloc_enabled is
 *   set, every block returned by malloc is filled with 0xFF bytes (a NaN when
 *   read as a double), like valgrind --malloc-fill=0xff.
 *
 *   The test executable defines malloc itself, so the calls the solver library
 *   makes resolve to it (symbol interposition). That only works with glibc;
 *   elsewhere fill_malloc_available() is 0 and the test is skipped.
 */

#include <stddef.h>
#include <string.h>

int fill_malloc_enabled = 0;

#if defined(__GLIBC__)

extern void *__libc_malloc(size_t size);

void *malloc(size_t size)
{
    void *p = __libc_malloc(size);
    if (fill_malloc_enabled && p) memset(p, 0xFF, size);
    return p;
}

int fill_malloc_available(void) { return 1; }

#else

int fill_malloc_available(void) { return 0; }

#endif
