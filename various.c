/* Copyright (c) 2016, 2018 Markus Uhlin <markus.uhlin@bredband.net>
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

#include <sys/stat.h>

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>

#include "log.h"
#include "various.h"

/**
 * Check if a file exists. It returns true for any filetype, even a
 * directory.
 *
 * @param path Path to file
 * @return true or false
 */
bool
file_exists(const char *path)
{
    struct stat sb;

    return path != NULL && *path != '\0' && stat(path, &sb) == 0;
}

/**
 * Check if a file is a directory. If path is either NULL or an empty
 * string it returns false.
 *
 * @param path Path to file
 * @return true or false
 */
bool
is_directory(const char *path)
{
    struct stat sb;

    if (path == NULL || *path == '\0')
	return false;
    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
}

/**
 * Check if a string consists of digits only determined by isdigit().
 *
 * @param string The string to check
 * @return true or false
 */
bool
is_numeric(const char *string)
{
    const char *p;

    if (string == NULL || *string == '\0') {
	return false;
    }

    for (p = &string[0]; *p != '\0'; p++) {
	if (!isdigit(*p))
	    return false;
    }

    return true;
}

/**
 * Check for a regular file. If path is either NULL or an empty string
 * it returns false.
 *
 * @param path Path to file
 * @return true or false
 */
bool
is_regularFile(const char *path)
{
    struct stat sb;

    if (path == NULL || *path == '\0')
	return false;
    return stat(path, &sb) == 0 && S_ISREG(sb.st_mode);
}

/**
 * Convert a input string to all lowercase characters. strToLower()
 * modifies the input string and return its result.
 *
 * @param s Input string
 * @return The result
 */
char *
strToLower(char *s)
{
    size_t len = 0;

    if (s == NULL) {
	fatal(EINVAL, "strToLower");
    } else if (*s == '\0') {
	return s;
    } else {
	len = strlen(s);
    }

    for (char *p = &s[0]; p < &s[len]; p++) {
	if (isupper(*p))
	    *p = tolower(*p);
    }

    return s;
}

/**
 * Delete trailing whitespace characters determined by isspace().
 *
 * @param string Input string
 * @return The result
 */
char *
trim(char *string)
{
    if (string == NULL) {
	fatal(EINVAL, "trim");
    } else if (*string == '\0') {
	return string;
    } else {
	char *p;

	for (p = &string[strlen(string) - 1]; p >= &string[0]; p--) {
	    if (!isspace(*p))
		break;
	}

	*(p + 1) = '\0';
    }

    return string;
}

/**
 * Calculate elt_count * elt_size and return its result -- but check
 * for overflow.
 *
 * @param elt_count	Element count
 * @param elt_size	Element size
 * @return The product
 */
size_t
size_product(const size_t elt_count, const size_t elt_size)
{
    if (elt_count > 0 && SIZE_MAX / elt_count < elt_size)
	fatal(ERANGE, "size_product: fatal: numerical result out of range");
    return (elt_count * elt_size);
}

/**
 * Toggle echo ON/OFF.
 *
 * @param state Set echoing to this state
 * @return Void
 */
void
toggle_echo(on_off_t state)
{
    static bool initialized = false;
    static struct termios term_attrs = {};

    if (!initialized) {
	if (tcgetattr(STDIN_FILENO, &term_attrs) != 0)
	    fatal(errno, "toggle_echo: tcgetattr");
	initialized = true;
    }

    switch (state) {
    case ON:
	if (! (term_attrs.c_lflag & ECHO)) {
	    term_attrs.c_lflag |= ECHO;
	    if (tcsetattr(STDIN_FILENO, TCSANOW, &term_attrs) != 0)
		fatal(errno, "toggle_echo: tcsetattr");
	}
	break;
    case OFF:
	if (term_attrs.c_lflag & ECHO) {
	    term_attrs.c_lflag &= ~ECHO;
	    if (tcsetattr(STDIN_FILENO, TCSANOW, &term_attrs) != 0)
		fatal(errno, "toggle_echo: tcsetattr");
	}
	break;
    default:
	assert(false); /* Shouldn't be reached. */
    }
}
