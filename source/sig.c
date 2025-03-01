/* Copyright (c) 2016-2023 Markus Uhlin <markus.uhlin@icloud.com>
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

#include <signal.h>
#include <syslog.h>
#include <unistd.h>

#include "daemonize.h"
#include "log.h"
#include "main.h"
#include "network.h"
#include "settings.h"
#include "sig.h"

static struct sig_message_tag {
	int	 num;     /**< Signal number */
	char	*num_str; /**< Official name */
	bool	 ignore;  /**< Ignore this signal if it arrives? */
	char	*msg;     /**< Signal description */
} sig_message[] = {
	{ SIGABRT,  "SIGABRT",  false, "Abnormal termination"           },
	{ SIGBUS,   "SIGBUS",   false, "Bus error (bad memory access)"  },
	{ SIGFPE,   "SIGFPE",   false, "Floating point exception"       },
	{ SIGILL,   "SIGILL",   false, "Illegal Instruction"            },
	{ SIGINT,   "SIGINT",   false, "Interrupt program"              },
	{ SIGSEGV,  "SIGSEGV",  false, "Invalid memory reference"       },
	{ SIGSYS,   "SIGSYS",   false, "Bad argument to routine"        },
	{ SIGTERM,  "SIGTERM",  false, "Termination signal"             },
	{ SIGXCPU,  "SIGXCPU",  false, "CPU time limit exceeded"        },
	{ SIGXFSZ,  "SIGXFSZ",  false, "File size limit exceeded"       },
	{ SIGHUP,   "SIGHUP",   true,  "Terminal line hangup"           },
	{ SIGPIPE,  "SIGPIPE",  true,  "Write on a pipe with no reader" },
	{ SIGQUIT,  "SIGQUIT",  true,  "Quit program"                   },
#ifdef SIGWINCH
	{ SIGWINCH, "SIGWINCH", true,  "Window resize signal"           },
#endif
};

static void
handle_signals(int signum)
{
	program_clean_up();

	for (struct sig_message_tag *ssp = &sig_message[0];
	    ssp < &sig_message[nitems(sig_message)];
	    ssp++) {
		if (ssp->num == signum) {
			log_warn(0, "Received signal %d (%s): %s", ssp->num,
			    ssp->num_str, ssp->msg);
		}
	}

	_exit(1);
}

void
block_signals(void)
{
	sigset_t set;

	(void) sigemptyset(&set);

	for (struct sig_message_tag *ssp = &sig_message[0];
	    ssp < &sig_message[nitems(sig_message)];
	    ssp++) {
		if (ssp->ignore)
			(void) sigaddset(&set, ssp->num);
	}
	if (sigprocmask(SIG_BLOCK, &set, NULL) != 0)
		fatal(errno, "%s: pthread_sigmask", __func__);
}

/**
 * This function is called whenever the program exits, no matter if it
 * was an error that caused it, or if the program exited normally.
 */
void
program_clean_up(void)
{
	net_deinit();
	destroy_config_custom_values();

	if (g_conf_read)
		log_msg("%s %s has exited", g_programName, g_programVersion);
	if (g_lockfile_fd != -1)
		close(g_lockfile_fd);
	if (g_log_to_syslog)
		closelog();
}

/**
 * Initialize signal handling for the program. For example decide what
 * to do if the OS sends SIGSEGV (invalid memory reference) to the
 * program.
 *
 * @return 0 on success, and -1 on failure
 */
int
sighand_init(void)
{
	sigset_t		 set;
	struct sigaction	 act = { 0 };

	(void) sigfillset(&set);
	(void) sigfillset(&act.sa_mask);
	act.sa_flags = 0;

	if (sigprocmask(SIG_SETMASK, &set, NULL) != 0) {
		log_warn(errno, "%s: SIG_SETMASK", __func__);
		return -1;
	}

	for (struct sig_message_tag *ssp = &sig_message[0];
	    ssp < &sig_message[nitems(sig_message)];
	    ssp++) {
		if (ssp->ignore) {
			act.sa_handler = SIG_IGN;
		} else {
			act.sa_handler = handle_signals;
		}
		if (sigaction(ssp->num, &act, NULL) != 0) {
			log_warn(errno, "%s: sigaction failed on signal %d "
			    "(%s)", __func__, ssp->num, ssp->num_str);
			return -1;
		}
	}

	(void) sigemptyset(&set);

	if (sigprocmask(SIG_SETMASK, &set, NULL) != 0) {
		log_warn(errno, "%s: SIG_SETMASK", __func__);
		return -1;
	}

	return 0;
}
