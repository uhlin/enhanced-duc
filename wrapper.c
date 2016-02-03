/* Copyright (c) 2015 Markus Uhlin <markus.uhlin@bredband.net>
   All rights reserved.

   Permission to use, copy, modify, and distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
   AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
   PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE. */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "wrapper.h"

void *xmalloc(size_t size)
{
    void *vp;

    if (size == 0) {
	log_die(EINVAL, "xmalloc: invalid argument -- zero size");
    }

    if ((vp = malloc(size)) == NULL) {
	log_die(ENOMEM, "xmalloc: error allocating %zu bytes", size);
    }

    return (vp);
}

void *xcalloc(size_t elt_count, size_t elt_size)
{
    void *vp;

    if (elt_count == 0) {
	log_die(EINVAL, "xcalloc: invalid argument: element count is zero");
    } else if (elt_size == 0) {
	log_die(EINVAL, "xcalloc: invalid argument: element size is zero");
    } else if (SIZE_MAX / elt_count < elt_size) {
	log_die(0, "xcalloc: integer overflow");
    } else {
	if ((vp = calloc(elt_count, elt_size)) == NULL)
	    log_die(ENOMEM, "xcalloc: out of memory (allocating %zu bytes)", (elt_count * elt_size));
    }

    return (vp);
}

void *xrealloc(void *ptr, size_t newSize)
{
    void *newPtr;

    if (ptr == NULL) {
	log_die(EINVAL, "xrealloc: invalid argument: a null pointer was passed");
    } else if (newSize == 0) {
	log_die(EINVAL, "xrealloc: invalid argument: zero size  --  use free");
    } else {
	if ((newPtr = realloc(ptr, newSize)) == NULL)
	    log_die(errno, "xrealloc: error changing memory block to %zu bytes", newSize);
    }

    return (newPtr);
}

char *xstrdup(const char *s)
{
    size_t	 sz	       = 0;
    char	*s_copy	       = NULL;
    int		 chars_printed = -1;
    
    if (s == NULL) {
	log_die(EINVAL, "xstrdup: invalid argument");
    } else {
	sz = strlen(s) + 1;
    }

    if ((s_copy = malloc(sz)) == NULL) {
	log_die(ENOMEM, "xstrdup: error allocating %zu bytes", sz);
    }

    if ((chars_printed = snprintf(s_copy, sz, "%s", s)) == -1 || (size_t) chars_printed >= sz) {
	log_die(errno, "xstrdup: snprintf error (chars_printed = %d)", chars_printed);
    }
    
    return (s_copy);
}

char *Strdup_printf(const char *format, ...)
{
    int my_vasprintf(char **ret, const char *format, va_list ap);
    va_list	 ap;
    int		 chars_printed;
    char	*ret;

    va_start(ap, format);
    chars_printed = my_vasprintf(&ret, format, ap);
    va_end(ap);

    if (chars_printed < 0) {
	log_die(errno, "Strdup_printf: fatal error");
    }
    
    return (ret);
}
