#ifndef VARIOUS_H
#define VARIOUS_H

#include <stdbool.h>
#include <string.h> /* strcmp */

#include "def.h" /* PTR_ARGS_NONNULL etc */

typedef enum { ON, OFF } on_off_t;

bool    file_exists    (const char *path);
bool    is_directory   (const char *path);
bool    is_numeric     (const char *);
bool    is_regularFile (const char *path);
char   *strToLower     (char *);
char   *trim           (char *);
size_t  size_product   (const size_t elt_count, const size_t elt_size);
void    toggle_echo    (on_off_t);

#if defined(HAVE_STRLCPY) && defined(HAVE_STRLCAT)
#define duc_strlcpy strlcpy
#define duc_strlcat strlcat
#endif

/* XXX: If the prefix duc_ is removed due to HAVE_STRLCPY and HAVE_STRLCAT these
        won't conflict the declarations in string.h: because _POSIX_C_SOURCE
        restricts the visibility... */
size_t	duc_strlcpy (char *dst, const char *src, size_t dsize) PTR_ARGS_NONNULL;
size_t	duc_strlcat (char *dst, const char *src, size_t dsize) PTR_ARGS_NONNULL;

static inline bool
Strings_match(const char *str1, const char *str2)
{
    return (strcmp(str1, str2) == 0);
}

#endif
