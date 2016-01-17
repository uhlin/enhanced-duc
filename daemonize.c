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

#include <errno.h>
#include <unistd.h>

#include "log.h"

#define FORK_FAILED -1
#define VALUE_CHILD_PROCESS 0

void Daemonize(void)
{
    switch (fork()) {
    case FORK_FAILED:
	log_die(errno, "Daemonize: Cannot fork");
    case VALUE_CHILD_PROCESS:
	if (setsid() == -1)
	    log_die(errno, "Daemonize: Trouble in becoming the session leader");
	break;
    default:
	_exit(0);
    }

    log_init(); /* Calls to the log functions before this log to stderr/stdout. */

    switch (redirect_standard_streams()) {
    case REDIR_STDERR_FAIL:
	log_warn(0, "Daemonize: Error redirecting stderr");
	break;
    case REDIR_STDIN_FAIL:
	log_warn(0, "Daemonize: Error redirecting stdin");
	break;
    case REDIR_STDOUT_FAIL:
	log_warn(0, "Daemonize: Error redirecting stdout");
	break;
    default:
    case REDIR_OK:
	log_debug("Daemonize: All standard IO-streams successfully redirected");
	break;
    }
}
