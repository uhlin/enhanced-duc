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
#include <sys/types.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "log.h"

#define FORK_FAILED -1
#define VALUE_CHILD_PROCESS 0

int g_lockfile_fd = -1;

static bool
is_already_running()
{
    int			 fd	   = -1;
    const char		*file_path = "/var/run/educ_noip.pid";
    const mode_t	 mode	   = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;	/* -rw-r--r-- */
    struct flock lock_ctx = {
	.l_type	  = F_WRLCK,
	.l_whence = SEEK_SET,
	.l_start  = 0,
	.l_len	  = 0,
	.l_pid	  = -1,
    };
    const int	OBTAIN_LOCK_ERR = -1;
    int		errno_save	= 0;

    if ((fd = open(file_path, O_RDWR | O_CREAT, mode)) == -1)
	fatal(errno, "is_already_running: can't open %s", file_path);
    if (fcntl(fd, F_SETLK, &lock_ctx) == OBTAIN_LOCK_ERR) {
	errno_save = errno;
	close(fd);
	if (errno_save == EACCES || errno_save == EAGAIN)
	    return (true);
	else
	    fatal(errno_save, "is_already_running: can't lock %s", file_path);
    }
    ftruncate(fd, 0);
    dprintf(fd, "%ld\n", (long int) getpid());
    g_lockfile_fd = fd;
    return (false);
}

/**
 * Run in the background. Detach the program from the controlling
 * terminal and continue execution...
 */
void
Daemonize()
{
    switch (fork()) {
    case FORK_FAILED:
	fatal(errno, "Daemonize: Cannot fork");
    case VALUE_CHILD_PROCESS:
	if (setsid() == -1)
	    fatal(errno, "Daemonize: Trouble in becoming the session leader");
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

    if (is_already_running())
	fatal(0, "Daemonize: FATAL: A copy of the daemon is already running!");
}
