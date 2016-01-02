#ifndef VARIOUS_H
#define VARIOUS_H

#include <stdbool.h>
#include <string.h> /* strcmp */

size_t  size_product   (const size_t elt_count, const size_t elt_size);
bool    file_exists    (const char *path);
bool    is_regularFile (const char *path);
bool    is_numeric     (const char *);
char   *trim           (char *);
char   *Strtolower     (char *);

static inline bool
Strings_match(const char *str1, const char *str2)
{
    return (strcmp(str1, str2) == 0);
}

#endif
