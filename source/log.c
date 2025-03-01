/* Copyright (c) 2015, 2018, 2021 Markus Uhlin <markus.uhlin@icloud.com>
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "log.h"

bool	 g_log_to_syslog = false;
bool	 g_debug_mode = false;

static void
log_doit(int errCode, int priority, const char *fmt, va_list ap)
{
	char buf[2000] = { '\0' };

	(void) vsnprintf(buf, sizeof buf, fmt, ap);

	if (errCode) {
		(void) snprintf(&buf[strlen(buf)], sizeof buf - strlen(buf),
		    ": %s", strerror(errCode));
	}

	if (g_log_to_syslog)
		syslog(priority, "%s", buf);
	else {
		FILE *stream = stderr;

		switch (priority) {
		case LOG_INFO:
		case LOG_DEBUG:
			stream = stdout;
			break;
		}

		(void) fputs(buf, stream);
		(void) fputc('\n', stream);
	}
}

/**
 * Redirect standard IO streams stderr, stdin and stdout to /dev/null.
 *
 * @return REDIR_OK, REDIR_STDERR_FAIL, REDIR_STDIN_FAIL or REDIR_STDOUT_FAIL
 */
redir_res_t
redirect_standard_streams(void)
{
	const char dev_null[] = "/dev/null";

	if (freopen(dev_null, "r+", stderr) == NULL)
		return REDIR_STDERR_FAIL;
	else if (freopen(dev_null, "r+", stdin) == NULL)
		return REDIR_STDIN_FAIL;
	else if (freopen(dev_null, "r+", stdout) == NULL)
		return REDIR_STDOUT_FAIL;
	return REDIR_OK;
}

/**
 * Handle fatal errors. This function calls abort(), i.e. it never
 * returns.
 *
 * @param code	Code passed to strerror()
 * @param fmt	Format control
 */
__dead void
fatal(int code, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	log_doit(code, LOG_ERR, fmt, ap);
	va_end(ap);

	abort();
}

/**
 * Log debug-level message
 */
void
log_debug(const char *fmt, ...)
{
	va_list ap;

	if (g_debug_mode) {
		va_start(ap, fmt);
		log_doit(0, LOG_DEBUG, fmt, ap);
		va_end(ap);
	}
}

/**
 * Initialize logging. Calls to the log functions before this log to
 * stderr/stdout.
 */
void
log_init(void)
{
	extern char	*__progname;
	static bool	 initialized = false;

	if (initialized)
		return;
	openlog(__progname, LOG_PID, LOG_DAEMON);
	g_log_to_syslog = initialized = true;
}

/**
 * Log informational message
 */
void
log_msg(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	log_doit(0, LOG_INFO, fmt, ap);
	va_end(ap);
}

/**
 * Log warning conditions
 *
 * @param code	Code passed to strerror()
 * @param fmt	Format control
 */
void
log_warn(int code, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	log_doit(code, LOG_WARNING, fmt, ap);
	va_end(ap);
}
