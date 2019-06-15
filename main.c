/* Copyright (c) 2016-2019 Markus Uhlin <markus.uhlin@bredband.net>
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

#if __OpenBSD__
#include <sys/param.h>
#endif
#include <sys/types.h>

#include <locale.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "base64.h"
#include "log.h"
#include "main.h"
#include "network.h"
#include "settings.h"
#include "sig.h"
#include "various.h"
#include "wrapper.h"

const char g_programName[]     = "Enhanced DUC";
const char g_programVersion[]  = "v2.1-dev";
const char g_programAuthor[]   = "Markus Uhlin";
const char g_maintainerEmail[] = "markus.uhlin@bredband.net";

char g_last_ip_addr[100] = "";

static const char *help_text[] = {
    "\n",
    "Options:\n",
    "  -h           Print help\n",
    "  -c           Create a config file by asking the user for input.\n",
    "               The user will be given the opportunity to choose a\n",
    "               location for the config file, i.e. where to create it.\n",
    "  -x <path>    Start the DUC with the config file specified by path\n",
    "  -D           Turn on debug mode\n",
    "  -o           Don't cycle, i.e. don't periodically check for IP\n",
    "               changes. Only update the hostname(s) once.\n",
    "  -B           Run in the background and act as a daemon\n",
    "\n",
};

static bool Cycle = true;

static const char enhanced_duc_user[] = DUC_USER;
static const char enhanced_duc_dir[]  = DUC_DIR;

static char *hostname_array[DUC_PERMITTED_HOSTS_LIMIT] = { NULL };

#define FOREACH_HOSTNAME()\
	for (char **ar_p = &hostname_array[0];\
	     ar_p < &hostname_array[ARRAY_SIZE(hostname_array)];\
	     ar_p++)

static void
process_options(int argc, char *argv[], struct program_options *po,
		char *ar, size_t ar_sz)
{
    const char opt_string[] = ":hcx:DoB";
    enum { MISSING_OPTARG = ':', UNRECOGNIZED_OPTION = '?' };
    int opt = -1;

    while ((opt = getopt(argc, argv, opt_string)) != -1) {
	switch (opt) {
	case MISSING_OPTARG:
	case UNRECOGNIZED_OPTION:
	    if (opt == MISSING_OPTARG) {
		fprintf(stderr, "Option '%c' requires an argument.\n", optopt);
	    } else {
		fprintf(stderr, "Unrecognized option '%c'.\n", optopt);
	    }
	    /*FALLTHROUGH*/
	case 'h':
	    po->want_usage = true;
	    return;
	case 'c':
	    po->want_create_config_file = true;
	    return;
	case 'x':
	    snprintf(ar, ar_sz, "%s", optarg);
	    break;
	case 'D':
	    po->want_debug = true;
	    break;
	case 'o':
	    po->want_update_once = true;
	    break;
	case 'B':
	    po->want_daemon = true;
	    break;
	}
    }
}

static NORETURN void
usage()
{
    extern char *__progname;
    char *msgVersion =
	strdup_printf("%s %s by %s\n",
		      g_programName, g_programVersion, g_programAuthor);
    char *msgUsage = strdup_printf("Usage: %s [OPTION] ...\n", __progname);
    const size_t ar_sz = ARRAY_SIZE(help_text);

    fputs(msgVersion, stderr);
    fputs(msgUsage, stderr);

    free(msgVersion);
    free(msgUsage);

    for (const char **ppcc = &help_text[0]; ppcc < &help_text[ar_sz]; ppcc++) {
	fputs(*ppcc, stderr);
    }

    exit(1);
}

static void
turn_on_debug_mode()
{
    g_debug_mode = true;
}

static void
set_cycle_off()
{
    Cycle = false;
}

static void
force_priv_drop()
{
    struct passwd *pw = getpwnam(enhanced_duc_user);

    log_msg("dropping root privileges...");

    if (pw == NULL)
	fatal(0, "getpwnam: no such user %s", enhanced_duc_user);
    else if (is_directory(enhanced_duc_dir) && chdir(enhanced_duc_dir) != 0)
	fatal(errno, "chdir %s", enhanced_duc_dir);
    else if (setgid(pw->pw_gid)  == -1) fatal(errno, "setgid");
    else if (setegid(pw->pw_gid) == -1) fatal(errno, "setegid");
    else if (setuid(pw->pw_uid)  == -1) fatal(errno, "setuid");
    else if (seteuid(pw->pw_uid) == -1) fatal(errno, "seteuid");
    else return;
}

/* ----------------------------------------------------------------- */

static void
hostname_array_init()
{
    FOREACH_HOSTNAME() {
	*ar_p = NULL;
    }
}

static void
hostname_array_assign()
{
    char *dump = xstrdup(setting("hostname"));
    const char legal_index[] =
	"abcdefghijklmnopqrstuvwxyz-0123456789.ABCDEFGHIJKLMNOPQRSTUVWXYZ|";

    if (strings_match(dump, "")) {
	fatal(EINVAL, "hostname_array_assign: no hostnames to update"
	    "  --  setting empty");
    }

    for (const char *cp = dump; *cp; cp++) {
	if (strchr(legal_index, *cp) == NULL)
	    fatal(0, "hostname_array_assign: invalid chars in setting: "
		  "first invalid char was '%c'...", *cp);
    }

    for (size_t hosts_assigned = 0;; hosts_assigned++) {
	char *token = strtok(hosts_assigned == 0 ? dump : NULL, "|");

	if (token && hosts_assigned < ARRAY_SIZE(hostname_array))
	    hostname_array[hosts_assigned] = xstrdup(token);
	else if (hosts_assigned == 0)
	    fatal(0, "hostname_array_assign: fatal: zero assigned hosts!");
	else {
	    log_debug("hostname_array_assign: "
		      "a total of %zu hosts were assigned!", hosts_assigned);
	    break;
	}
    }

    free(dump);
}

static void
flag_err_and_output_warning(bool *ok, const char *msg)
{
    *ok = false;
    log_warn(0, "%s", msg);
}

static int
send_update_request(const char *which_host, const char *to_ip)
{
    bool ok = true;
    char *agent = NULL;
    char *auth = NULL;
    char *s, *host, *unp;
    char buf[500] = "";

    s = host = unp = NULL;

    if (strings_match(to_ip, "WAN_address")) {
	s = strdup_printf("GET %s?hostname=%s HTTP/1.0",
	    UPDATE_SCRIPT, which_host);
    } else {
	s = strdup_printf("GET %s?hostname=%s&myip=%s HTTP/1.0",
	    UPDATE_SCRIPT, which_host, to_ip);
    }
    host = strdup_printf("Host: %s", setting("sp_hostname"));
    unp	 = strdup_printf("%s:%s", setting("username"), setting("password"));
    if (b64_encode((uint8_t *) unp, strlen(unp), buf, sizeof buf) < 0) {
	log_warn(EMSGSIZE, "send_update_request: b64_encode");
	ok = false;
	goto err;
    }
    auth  = strdup_printf("Authorization: Basic %s", buf);
    agent = strdup_printf("User-Agent: %s/%s %s",
			  g_programName, g_programVersion, g_maintainerEmail);

    log_debug("sending http get request");
    if (net_send("%s\r\n%s\r\n%s\r\n%s", s, host, auth, agent) != 0)
	ok = false;

  err:
    free_not_null(s);
    free_not_null(host);
    free_not_null(unp);
    free_not_null(auth);
    free_not_null(agent);
    return ok ? 0 : -1;
}

static int
store_server_resp_in_buffer(char **buf)
{
    const size_t sz = 2000;

    if (*buf) {
	free(*buf);
	*buf = NULL;
    }

    *buf = xcalloc(sz, 1);

    if (net_recv(*buf, sz) == -1)
	return -1;

    char concatSource[500] = { '\0' };

    (void) net_recv(concatSource, ARRAY_SIZE(concatSource));

    if (!strings_match(concatSource, "") &&
	strlcat(*buf, concatSource, sz) >= sz) {
	log_warn(0, "warning: store_server_resp_in_buffer: strlcat: "
	    "response truncated!");
	return -1;
    }

    return 0;
}

static response_code_t
server_response(const char *buf)
{
    char *dump, *cp, *r;
    const struct responses_tag {
	char		*str;
	response_code_t	 code;
    } responses[] = {
	{ "good",     CODE_GOOD       },
	{ "nochg",    CODE_NOCHG      },
	{ "nohost",   CODE_NOHOST     },
	{ "badauth",  CODE_BADAUTH    },
	{ "badagent", CODE_BADAGENT   },
	{ "!donator", CODE_NOTDONATOR },
	{ "abuse",    CODE_ABUSE      },
	{ "911",      CODE_EMERG      },
    };
    const size_t ar_sz = ARRAY_SIZE(responses);
    const struct responses_tag *ar_p = &responses[0];

    dump = cp = r = NULL;

    if (buf == NULL || strings_match(buf, "")) {
	return CODE_UNKNOWN;
    } else {
	char *buf_copy = xstrdup(buf);
	char *buf_ptr = NULL;

	while ((buf_ptr = strpbrk(buf_copy, "\r\n")) != NULL) {
	    if (*buf_ptr == '\r') {
		*buf_ptr = 'R';
	    } else {
		*buf_ptr = 'N';
	    }
	}

	log_debug("server_response: the buffer looks like this: %s", buf_copy);
	free(buf_copy);
    }

    dump = trim(xstrdup(buf));
    if ((cp = strrchr(dump, '\n')) == NULL) {
	free(dump);
	return CODE_UNKNOWN;
    }
    r = strToLower(xstrdup(++cp));
    free(dump);
    log_debug("server_response: r = \"%s\"", r);

    if (!strncmp(r, "good ", 5)) {
	free(r);
	return CODE_GOOD;
    } else if (!strncmp(r, "nochg ", 6)) {
	free(r);
	return CODE_NOCHG;
    } else {
	for (; ar_p < &responses[ar_sz]; ar_p++) {
	    if (strings_match(ar_p->str, r)) {
		free(r);
		return ar_p->code;
	    }
	}
    }

    free(r);
    return CODE_UNKNOWN;
}

static bool
update_host(const char *which_host, const char *to_ip,
	    bool *updateRequestAfter30Min)
{
    bool ok = true;
    char *buf = NULL;

    if (which_host == NULL || to_ip == NULL || updateRequestAfter30Min == NULL)
	fatal(EINVAL, "update_host() fatal error!");

    if (net_connect() == -1 || send_update_request(which_host, to_ip) == -1 ||
	store_server_resp_in_buffer(&buf) == -1) {
	ok = false;
	goto err;
    }

    switch (server_response(buf)) {
    case CODE_GOOD:
	log_msg("dns hostname update successful");
	break;
    case CODE_NOCHG:
	log_msg("ip address is current");
	break;
    case CODE_NOHOST:
	fatal(0, "Hostname supplied does not exist under specified account.");
	break;
    case CODE_BADAUTH:
	fatal(0, "Invalid username password combination.");
	break;
    case CODE_BADAGENT:
	fatal(0, "Bad agent? I don't think so! "
	    "But the server is always right. Dying...");
	break;
    case CODE_NOTDONATOR:
	fatal(0, "Bad update request. Feature not available.");
	break;
    case CODE_ABUSE:
	fatal(0, "Username blocked due to abuse.");
	break;
    case CODE_EMERG:
	if (Cycle)
	    log_warn(0, "fatal error on the server side "
		"(will retry update after 30 minutes)");
	else
	    log_warn(0, "fatal error on the server side");
	*updateRequestAfter30Min = true;
	break;
    default:
    case CODE_UNKNOWN:
	flag_err_and_output_warning(&ok, "Unknown server response!");
	break;
    }

  err:
    net_disconnect();
    free_not_null(buf);
    return (ok);
}

static void
hostname_array_destroy()
{
    FOREACH_HOSTNAME() {
	free_not_null(*ar_p);
	*ar_p = NULL;
    }
}

static void
start_update_cycle()
{
    hostname_array_init();

#if defined(OpenBSD) && OpenBSD >= 201605
    if (pledge("stdio inet dns", NULL) == -1)
	fatal(errno, "pledge");
    log_msg("forced into a restricted service operating mode (good)");
#endif

    do {
	bool updateRequestAfter30Min = false;

	if (!Cycle || net_check_for_ip_change() == IP_HAS_CHANGED) {
	    hostname_array_assign();

	    FOREACH_HOSTNAME() {
		if (! (*ar_p))
		    break;
		if (updateRequestAfter30Min)
		    break;

		log_msg("trying to update %s", *ar_p);

		if (!update_host(*ar_p, setting("ip_addr"),
				 &updateRequestAfter30Min))
		    log_warn(0, "failed to update hostname");
	    }

	    hostname_array_destroy();
	}

	if (Cycle) {
	    struct integer_unparse_context ctx = {
		.setting_name = "update_interval_seconds",
		.lo_limit     = 600,	/* 10 minutes */
		.hi_limit     = 172800,	/* 2 days */
		.fallback_val = 1800,	/* 30 minutes */
	    };
	    struct timespec ts = {
		.tv_sec	 = ((updateRequestAfter30Min)
			    ? 1800
			    : setting_integer_unparse(&ctx)),
		.tv_nsec = 0,
	    };

	    log_debug("sleeping for %ld seconds", (long int) ts.tv_sec);
	    nanosleep(&ts, NULL);
	}
    } while (Cycle);
}

#ifndef UNIT_TESTING
int
main(int argc, char *argv[])
{
    char conf[DUC_PATH_MAX] =
	"/etc/enhanced-duc.conf";
    struct program_options opt = {
	.want_usage		 = false,
	.want_create_config_file = false,
	.want_debug		 = false,
	.want_update_once	 = false,
	.want_daemon		 = false,
    };

    if (sigHand_init() == -1)
	log_warn(0, "Initialization of signal handling failed");
    if (atexit(program_clean_up) == -1)
	log_warn(errno, "Failed to register a clean up function");

    setlocale(LC_ALL, "");
    process_options(argc,argv,&opt,&conf[0],sizeof conf); /*Always successful*/

    if (opt.want_usage) {
	usage();
	/* NOTREACHED */
    }
    if (opt.want_create_config_file) {
	char *path = NULL;

	while (path = get_answer("Create where?", TYPE_STRING, &conf[0]),
	       path == NULL)
	    /* continue */;
	create_config_file(path);
	free(path);
	exit(0);
    }
    if (opt.want_debug)
	turn_on_debug_mode();
    if (opt.want_update_once)
	set_cycle_off();
    if (opt.want_daemon) {
	void Daemonize(void);

	/*
	 * Detach the program from the controlling terminal and continue
	 * execution...
	 */
	Daemonize();
    }

    log_msg("%s %s has started", g_programName, g_programVersion);
    log_msg("reading %s...", conf);
    read_config_file(conf);
    check_some_settings_strictly();

    /* Drop root privileges. */
    if (geteuid() == UID_SUPER_USER) {
	force_priv_drop();
	log_msg("EUID = %ld", (long int) geteuid());
    }

    net_init();
    start_update_cycle();

    /* Exit program */
    return 0;
}
#endif /* UNIT_TESTING */
/* EOF */
