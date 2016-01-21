#ifndef VARIOUS_H
#define VARIOUS_H

#include <stdbool.h>
#include <string.h> /* strcmp */

#include "def.h" /* PTR_ARGS_NONNULL etc */

size_t  size_product   (const size_t elt_count, const size_t elt_size);
bool    file_exists    (const char *path);
bool    is_regularFile (const char *path);
bool    is_directory   (const char *path);
bool    is_numeric     (const char *);
char   *trim           (char *);
char   *Strtolower     (char *);

size_t	duc_strlcpy (char *dst, const char *src, size_t dsize) PTR_ARGS_NONNULL;
size_t	duc_strlcat (char *dst, const char *src, size_t dsize) PTR_ARGS_NONNULL;

static inline bool
Strings_match(const char *str1, const char *str2)
{
    return (strcmp(str1, str2) == 0);
}

#endif
