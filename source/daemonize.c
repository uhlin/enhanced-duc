/* Copyright (c) 2016-2021 Markus Uhlin <markus.uhlin@bredband.net>
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
#include <sys/types.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "daemonize.h"
#include "log.h"

const char	g_lockfile_path[] = "/var/run/enhanced-duc.pid";
int		g_lockfile_fd = -1;

/*
 * mode: -rw-r--r--
 */
static const mode_t mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

static bool
is_already_running(void)
{
	int	errno_save = 0;
	int	fd = -1;
	struct flock lock_ctx = {
		.l_type   = F_WRLCK,
		.l_whence = SEEK_SET,
		.l_start  = 0,
		.l_len    = 0,
		.l_pid    = -1,
	};

	if ((fd = open(g_lockfile_path, O_RDWR | O_CREAT, mode)) == -1) {
		fatal(errno, "is_already_running: can't open %s",
		    g_lockfile_path);
	}

	if (fcntl(fd, F_SETLK, &lock_ctx) == FAILED_TO_ACQUIRE_LOCK) {
		errno_save = errno;
		(void) close(fd);

		if (errno_save == EACCES || errno_save == EAGAIN)
			return true;
		else {
			fatal(errno_save, "is_already_running: can't lock %s",
			    g_lockfile_path);
		}
	}

	(void) ftruncate(fd, 0);
	(void) dprintf(fd, "%ld\n", (long int) getpid());
	g_lockfile_fd = fd;
	return false;
}

/**
 * Run in the background. Detach the program from the controlling
 * terminal and continue execution...
 */
void
daemonize(void)
{
	switch (fork()) {
	case FORK_FAILED:
		fatal(errno, "daemonize: cannot fork");
		break;
	case VALUE_CHILD_PROCESS:
		if (setsid() == -1)
			fatal(errno, "daemonize: trouble in becoming "
			    "the session leader");
		break;
	default:
		_exit(0);
	}

	/* Calls to the log functions before this log to stderr/stdout. */
	log_init();

	switch (redirect_standard_streams()) {
	case REDIR_STDERR_FAIL:
		log_warn(0, "daemonize: error redirecting stderr");
		break;
	case REDIR_STDIN_FAIL:
		log_warn(0, "daemonize: error redirecting stdin");
		break;
	case REDIR_STDOUT_FAIL:
		log_warn(0, "daemonize: error redirecting stdout");
		break;
	default:
	case REDIR_OK:
		log_debug("daemonize: all standard io-streams successfully "
		    "redirected");
		break;
	}

	if (is_already_running()) {
		fatal(0, "daemonize: fatal: a copy of the daemon is already "
		    "running!");
	}
}
