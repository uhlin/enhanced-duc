/* Copyright (c) 2012-2025 Markus Uhlin <markus.uhlin@icloud.com>
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
#include "wrapper.h"

const char g_fgets_nullret_err1[70] = "error: fgets() returned null and the "
    "error indicator is set";
const char g_fgets_nullret_err2[70] = "error: fgets() returned null for an "
    "unknown reason";

static const char ArgBegin = '"';
static const char ArgEnd = '"';
static const char CommentChar = '#';

static const size_t	identifier_maxSize = 64;
static const size_t	argument_maxSize = 512;

/**
 * Copy identifier
 */
static char *
copy_identifier(const char *&id)
{
	size_t	 count = identifier_maxSize;
	char	*dest_buf = new char[count + 1];
	char	*dest = addrof(dest_buf[0]);

	while ((isalnum(*id) || *id == '_') && count > 1)
		*dest++ = *id++, count--;

	*dest = '\0';

	if (count == 1)
		fatal(EOVERFLOW, "%s: fatal: string was truncated", __func__);
	return dest_buf;
}

/**
 * Copy argument
 */
//lint -sem(copy_argument, r_null)
static char *
copy_argument(const char *&arg)
{
	bool	 inside_arg = true;
	size_t	 count = argument_maxSize;
	char	*dest_buf = new char[count + 1];
	char	*dest = addrof(dest_buf[0]);

	while (*arg && count > 1) {
		if (*arg == ArgEnd) {
			inside_arg = false;
			arg++;
			break;
		}

		*dest++ = *arg++, count--;
	}

	*dest = '\0';

	if (inside_arg && count == 1)
		fatal(EOVERFLOW, "%s: fatal: string was truncated", __func__);
	if (inside_arg) {
		delete[] dest_buf;
		return nullptr;
	}
	return dest_buf;
}

static void
clean_up(char *id, char *arg)
{
	delete[] id;
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
	char	*id = nullptr;
	char	*arg = nullptr;

	if (in == nullptr)
		fatal(EINVAL, "%s", __func__);

	try {
		const char *cp = addrof(in->line[0]);

		if (!isalnum(*cp) && *cp != '_')
			throw std::runtime_error("unexpected leading "
			    "character");
		id = copy_identifier(cp);
		eatwhite(&cp);
		if (*cp++ != '=') {
			throw std::runtime_error("expected assignment "
			    "operator");
		}

		eatwhite(&cp);
		if (*cp++ != ArgBegin)
			throw std::runtime_error("expected arg begin");
		else if ((arg = copy_argument(cp)) == nullptr)
			throw std::runtime_error("unterminated argument");

		eatwhite(&cp);
		if (*cp++ != ';')
			throw std::runtime_error("no line terminator!");

		eatwhite(&cp);
		if (*cp && *cp != CommentChar) {
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
	} catch (const std::bad_alloc &e) {
		std::cerr << "out of memory: " << e.what() << '\n';
		clean_up(id, arg);
		abort();
	} catch (const std::runtime_error &e) {
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
	} catch (...) {
		std::cerr << "unknown exception  --  should not be reached\n";
		clean_up(id, arg);
		abort();
	}

	clean_up(id, arg);
}

void
Interpreter_processAllLines(FILE *fp, const char *path, Interpreter_vFunc func1,
    Interpreter_instFunc func2)
{
	char		buf[MAXLINE] = { '\0' };
	long int	line_num = 0;

	while (fgets(buf, sizeof buf, fp) != nullptr) {
		char			*line;
		const char		*cp;
		struct Interpreter_in	 in;

		cp = &buf[0];
		eatwhite(&cp);
		if (strings_match(cp, "") || *cp == CommentChar) {
			line_num++;
			continue;
		}

		line = trim(xstrdup(cp));

		in.path			= const_cast<char *>(path);
		in.line			= line;
		in.line_num		= ++line_num;
		in.validator_func	= func1;
		in.install_func		= func2;
		Interpreter(&in);

		free(line);
	}
}
