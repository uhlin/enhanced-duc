#ifndef WRAPPER_H
#define WRAPPER_H

#include "ducdef.h"

#include <stdlib.h>

__DUC_BEGIN_DECLS
char	*strdup_printf(const char *, ...) PRINTFLIKE(1);
char	*xstrdup(const char *);
void	*xcalloc(size_t elt_count, size_t elt_size);
void	*xmalloc(size_t);
void	*xrealloc(void *ptr, size_t newSize);
__DUC_END_DECLS

static inline void
free_not_null(void *ptr)
{
	if (ptr != NULL)
		free(ptr);
}

#endif
