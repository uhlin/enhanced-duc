/* Copyright (c) 2012-2021 Markus Uhlin <markus.uhlin@bredband.net>
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

#include <iostream>
#include <stdexcept>

#include "interpreter.h"
#include "log.h"
#include "various.h"

/* Set to 0 to turn off this feature. */
#define IGNORE_UNRECOGNIZED_IDENTIFIERS 1

static const size_t identifier_maxSize = 64;
static const size_t argument_maxSize = 512;

/**
 * Copy identifier
 */
static char *
copy_identifier(const char *&id)
{
    size_t  count    = identifier_maxSize;
    char   *dest_buf = new char[count + 1];
    char   *dest     = &dest_buf[0];

    while ((isalnum(*id) || *id == '_') && count > 1) {
	*dest++ = *id++, count--;
    }

    *dest = '\0';

    if (count == 1)
	fatal(EOVERFLOW, "In copy_identifier: fatal: string was truncated!");

    return dest_buf;
}

/**
 * Copy argument
 */
/*lint -sem(copy_argument, r_null) */
static char *
copy_argument(const char *&arg)
{
    bool    inside_arg = true;
    size_t  count      = argument_maxSize;
    char   *dest_buf   = new char[count + 1];
    char   *dest       = &dest_buf[0];

    while (*arg && count > 1) {
	if (*arg == '\"') {
	    inside_arg = false;
	    arg++;
	    break;
	}

	*dest++ = *arg++, count--;
    }

    *dest = '\0';

    if (inside_arg && count == 1)
	fatal(EOVERFLOW, "In copy_argument: fatal: string was truncated!");

    if (inside_arg) {
	delete[] dest_buf;
	return NULL;
    }

    return dest_buf;
}

static void
clean_up(char *id, char *arg)
{
    if (id)
	delete[] id;
    if (arg)
	delete[] arg;
}

/**
 * Interpreter
 *
 * @param in Context structure
 * @return Void
 *
 * An interpreter for configuration files. The context structure
 * contains the data to be passed to the interpreter.
 */
void
Interpreter(const struct Interpreter_in *in)
{
	char	*id = NULL;
	char	*arg = NULL;

	try {
		if (in == NULL)
			throw std::runtime_error("null input");

		const char *cp = addrof(in->line[0]);

		if (!isalnum(*cp) && *cp != '_')
			throw std::runtime_error("unexpected leading "
			    "character");
		id = copy_identifier(cp);
		adv_while_isspace(&cp);
		if (*cp++ != '=') {
			throw std::runtime_error("expected assignment "
			    "operator");
		}

		adv_while_isspace(&cp);
		if (*cp++ != '\"')
			throw std::runtime_error("expected string");
		else if ((arg = copy_argument(cp)) == NULL)
			throw std::runtime_error("unterminated argument");

		adv_while_isspace(&cp);
		if (*cp++ != ';')
			throw std::runtime_error("no line terminator!");

		adv_while_isspace(&cp);
		if (*cp && *cp != '#') {
			throw std::runtime_error("implicit data after "
			    "line terminator!");
		} else if (!(in->validator_func(id))) {
#if IGNORE_UNRECOGNIZED_IDENTIFIERS
			/* ignore */;
#else
			throw std::runtime_error("no such identifier");
#endif
		} else if ((errno = in->install_func(id, arg)) != 0) {
			throw std::runtime_error("install error");
		}
	} catch (std::runtime_error& e) {
		std::cerr << '\t' << in->line << '\n';

		if (strings_match(e.what(), "install error")) {
			log_warn(errno, "%s:%ld: error: "
			    "install_func returned %d", in->path, in->line_num,
			    errno);
		} else {
			log_warn(0, "%s:%ld: error: %s", in->path, in->line_num,
			    e.what());
		}

		clean_up(id, arg);
		abort();
	}

	clean_up(id, arg);
}
