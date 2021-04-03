#ifndef WRAPPER_H
#define WRAPPER_H

#include "ducdef.h"

#include <stdlib.h>

char *strdup_printf (const char *format, ...) PRINTFLIKE(1);
char *xstrdup       (const char *string);
void *xcalloc       (size_t elt_count, size_t elt_size);
void *xmalloc       (size_t);
void *xrealloc      (void *ptr, size_t newSize);

static inline void
free_not_null(void *ptr)
{
    if (ptr != NULL)
	free(ptr);
}

#endif
