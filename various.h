#ifndef VARIOUS_H
#define VARIOUS_H

#include <stdarg.h>
#include <stdbool.h>
#include <string.h> /* strcmp */

#include "ducdef.h" /* PTR_ARGS_NONNULL etc */
#include "enhanced-duc-config.h"

typedef enum { ON, OFF } on_off_t;

__DUC_BEGIN_DECLS
bool	 file_exists(const char *);
bool	 is_directory(const char *);
bool	 is_numeric(const char *);
bool	 is_regularFile(const char *);
char	*strToLower(char *);
char	*trim(char *);
int	 my_vasprintf(char **ret, const char *format, va_list);
size_t	 size_product(const size_t elt_count, const size_t elt_size);
void	 toggle_echo(on_off_t);
__DUC_END_DECLS

#if HAVE_STRLCPY == 0 || !defined(_BSD_SOURCE)
__DUC_BEGIN_DECLS
size_t strlcpy(char *, const char *, size_t) PTR_ARGS_NONNULL;
__DUC_END_DECLS
#endif /* HAVE_STRLCPY == 0 */

#if HAVE_STRLCAT == 0 || !defined(_BSD_SOURCE)
__DUC_BEGIN_DECLS
size_t strlcat(char *, const char *, size_t) PTR_ARGS_NONNULL;
__DUC_END_DECLS
#endif /* HAVE_STRLCAT == 0 */

static inline bool
strings_match(const char *str1, const char *str2)
{
	return (strcmp(str1, str2) == 0);
}

#endif
