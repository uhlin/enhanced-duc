/* Copyright (c) 2016 Markus Uhlin <markus.uhlin@bredband.net>
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

void toggle_echo(on_off_t state)
{
    static bool initialized = false;
    static struct termios term_attrs = {};

    if (!initialized) {
	if (tcgetattr(STDIN_FILENO, &term_attrs) != 0)
	    log_die(errno, "toggle_echo: tcgetattr error");
	initialized = true;
    }

    switch (state) {
    case ON:
	if (! (term_attrs.c_lflag & ECHO)) {
	    term_attrs.c_lflag |= ECHO;
	    if (tcsetattr(STDIN_FILENO, TCSANOW, &term_attrs) != 0)
		log_die(errno, "toggle_echo: tcsetattr error");
	}
	break;
    case OFF:
	if (term_attrs.c_lflag & ECHO) {
	    term_attrs.c_lflag &= ~ECHO;
	    if (tcsetattr(STDIN_FILENO, TCSANOW, &term_attrs) != 0)
		log_die(errno, "toggle_echo: tcsetattr error");
	}
	break;
    default:
	assert(false); /* Shouldn't be reached. */
    }
}

size_t size_product(const size_t elt_count, const size_t elt_size)
{
    if (elt_count > 0 && SIZE_MAX / elt_count < elt_size)
	log_die(ERANGE, "size_product: FATAL: numerical result out of range");
    
    return (elt_count * elt_size);
}

bool file_exists(const char *path)
{
    struct stat sb;

    return path != NULL && *path != '\0' && stat(path, &sb) == 0;
}

bool is_regularFile(const char *path)
{
    struct stat sb;

    if (path == NULL || *path == '\0') return false;

    return stat(path, &sb) == 0 && S_ISREG(sb.st_mode);
}

bool is_directory(const char *path)
{
    struct stat sb;

    if (path == NULL || *path == '\0') return false;

    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
}

bool is_numeric(const char *string)
{
    const char *p;

    if (string == NULL || *string == '\0') {
	return false;
    }

    for (p = &string[0]; *p != '\0'; p++) {
	if (!isdigit(*p)) return false;
    }

    return true;
}

char *trim(char *string)
{
    if (string == NULL) {
	log_die(EINVAL, "trim error");
    } else if (*string == '\0') {
	return string;
    } else {
	char *p;
	
	for (p = &string[strlen(string) - 1]; p >= &string[0]; p--) {
	    if (!isspace(*p)) break;
	}
	
	*(p + 1) = '\0';
    }
    
    return string;
}

char *Strtolower(char *s)
{
    size_t len = 0;

    if (s == NULL) {
	log_die(EINVAL, "Strtolower error");
    } else if (*s == '\0') {
	return s;
    } else {
	len = strlen(s);
    }

    for (char *p = &s[0]; p < &s[len]; p++) {
	if (isupper(*p)) *p = tolower(*p);
    }

    return s;
}
