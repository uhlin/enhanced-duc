#ifndef VARIOUS_H
#define VARIOUS_H

#include <stdarg.h>
#include <stdbool.h>
#include <string.h> /* strcmp */

#include "def.h" /* PTR_ARGS_NONNULL etc */
#include "enhanced-duc-config.h"

typedef enum { ON, OFF } on_off_t;

bool    file_exists    (const char *path);
bool    is_directory   (const char *path);
bool    is_numeric     (const char *);
bool    is_regularFile (const char *path);
char   *strToLower     (char *);
char   *trim           (char *);
int     my_vasprintf   (char **ret, const char *format, va_list);
size_t  size_product   (const size_t elt_count, const size_t elt_size);
void    toggle_echo    (on_off_t);

#if HAVE_STRLCPY == 0
size_t strlcpy(char *dst, const char *src, size_t dsize) PTR_ARGS_NONNULL;
#endif

#if HAVE_STRLCAT == 0
size_t strlcat(char *dst, const char *src, size_t dsize) PTR_ARGS_NONNULL;
#endif

static inline bool
strings_match(const char *str1, const char *str2)
{
    return (strcmp(str1, str2) == 0);
}

#endif
