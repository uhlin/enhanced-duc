/* Copyright (c) 2016, 2021 Markus Uhlin <markus.uhlin@icloud.com>
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

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int
my_vasprintf(char **ret, const char *format, va_list ap)
{
	va_list ap_copy;
	int sz;

	if (ret == NULL || format == NULL) {
		errno = EINVAL;
		return -1;
	}

	va_copy(ap_copy, ap);
	sz = vsnprintf(NULL, 0, format, ap_copy);
	va_end(ap_copy);

	if (sz < 0) {
		*ret = NULL;
		errno = ENOSYS;
		return -1;
	} else {
		sz += 1; /* +1 for `\0'. */
	}

	if ((*ret = malloc(sz)) == NULL) {
		errno = ENOMEM;
		return -1;
	}

	return vsnprintf(*ret, sz, format, ap);
}
