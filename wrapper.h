#ifndef WRAPPER_H
#define WRAPPER_H

#include "def.h"

#include <stdlib.h>

void *xmalloc       (size_t);
void *xcalloc       (size_t elt_count, size_t elt_size);
void *xrealloc      (void *ptr, size_t newSize);
char *xstrdup       (const char *string);
char *Strdup_printf (const char *format, ...) PRINTFLIKE(1);

static inline void free_not_null(void *ptr)
{
    if (ptr != NULL)
	free(ptr);
}

#endif
