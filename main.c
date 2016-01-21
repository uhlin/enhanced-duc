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
#include <errno.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "def.h"
#include "interpreter.h"
#include "log.h"
#include "main.h"
#include "network.h"
#include "sig.h"
#include "various.h"
#include "wrapper.h"

#define SHOW_OPTIONS_FOR_MAINTAINERS	1
#define DUC_PERMITTED_HOSTS_LIMIT	10	/* Qty of hostnames allowed. */
#define DUC_PATH_MAX			500	/* Max bytes in a pathname. */
#define UID_SUPER_USER			0

const char g_programName[]     = "Enhanced DUC";
const char g_programVersion[]  = "v1.0";
const char g_programAuthor[]   = "Markus Uhlin";
const char g_maintainerEmail[] = "markus.uhlin@bredband.net";

static const char *help_text[] = {
    "\n",
    "Options:\n",
    "  -h           Print this help text.\n",
    "  -c           Create a config file and exit. Will prompt.\n",
    "  -x <path>    Load a config file from a custom location.\n",
#if SHOW_OPTIONS_FOR_MAINTAINERS
    "  -D           D for debug log/messages.\n",
#endif
    "  -B           Run in the background.\n",
    "\n",
};

static const char educ_noip_user[] = "nobody";
static const char educ_noip_dir[]  = "/tmp";

static char *hostname_array[DUC_PERMITTED_HOSTS_LIMIT] = {};

static struct config_default_values_tag {
    char		*setting_name;
    enum setting_type	 type;
    char		*value;
    char		*custom_val;
    char		*description;
} config_default_values[] = {
    { "username", TYPE_STRING, "ChangeMe", NULL,
      "Your username." },
    
    { "password", TYPE_STRING, "ChangeMe", NULL,
      "Your password." },

    { "hostname", TYPE_STRING, "host1.domain.com|host2.domain.com", NULL,
      "The hostname to be updated. Multiple hosts are separated with a vertical bar." },

    { "ip_addr", TYPE_STRING, "WAN_address", NULL,
      "Associate the hostname with this IP address. If the special value WAN_address\n"
      "is specified: the WAN address from which the update request is sent from is used." },

    { "sp_hostname", TYPE_STRING, "dynupdate.no-ip.com", NULL,
      "Service provider hostname. The http GET request is sent to this hostname." },

    { "port", TYPE_INTEGER, "80", NULL,
      "Connect to sp_hostname + this port." },

    { "update_interval_seconds", TYPE_INTEGER, "1800", NULL,
      "Update interval specified in seconds. If a value less than 600 is entered, the\n"
      "underlying code will fallback to 600 to avoid flooding the server with requests." },
};

static void             process_options       (int, char *[], struct program_options *, char *, size_t);
static void             usage                 (void) NORETURN;
static void             create_config_file    (const char *path);
static char            *get_answer            (const char *, enum setting_type, const char *);
static bool             is_setting_ok         (const char *value, enum setting_type);
static void             read_config_file      (const char *path);
static void             turn_on_debug_mode    (void);
static void             force_priv_drop       (void);
static bool             is_recognized_setting (const char *setting_name);
static int              install_setting       (const char *setting_name, const char *value);
static void             start_update_cycle          (void);
static void             hostname_array_init         (void);
static void             hostname_array_destroy      (void);
static void             hostname_array_assign       (void);
static bool             update_host                 (const char *which_host, const char *to_ip, bool *);
static int              send_update_request         (const char *which_host, const char *to_ip);
static int              store_server_resp_in_buffer (char **buf);
static response_code_t  server_response             (const char *buf);

int
main(int argc, char *argv[])
{
    struct program_options opt = {
	.want_usage		 = false,
	.want_create_config_file = false,
	.want_debug		 = false,
	.want_daemon		 = false,
    };
    char conf[DUC_PATH_MAX] =
	"/etc/educ_noip.conf"; /* process_options change the value if -x is passed. */
    
    if (sigHand_init() == -1)
	log_warn(0, "Initialization of signal handling failed");
    if (atexit(program_clean_up) == -1)
	log_warn(errno, "Failed to register a clean up function");
    
    process_options(argc, argv, &opt, &conf[0], sizeof conf); /* Always successful. */
    
    if (opt.want_usage)
	usage(); /* Doesn't return. */
    if (opt.want_create_config_file) {
	char *path = get_answer("Create where?", TYPE_STRING, &conf[0]);
	
	create_config_file(path);
	free(path);
	exit(0);
    }
    if (opt.want_debug)
	turn_on_debug_mode();
    if (opt.want_daemon) {
	extern void Daemonize(void);

	/* Detach the program from the controlling terminal and continue execution... */
	Daemonize();
    }
    
    log_msg("%s %s has started.", g_programName, g_programVersion);
    log_msg("Reading %s...", conf);
    read_config_file(conf);

    /* Drop root privileges. */
    if (geteuid() == UID_SUPER_USER) {
	force_priv_drop();
    }
    
    start_update_cycle();
    
    return 0;
}

static void
process_options(int argc, char *argv[], struct program_options *po, char *ar, size_t ar_sz)
{
    int opt = -1;
    const char opt_string[] = ":hcx:DB";
    enum { MISSING_OPTARG = ':', UNRECOGNIZED_OPTION = '?' };

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
	case 'B':
	    po->want_daemon = true;
	    break;
	}
    }
}

static void
usage(void)
{
    extern char		*__progname;
    char		*msgVersion = Strdup_printf("%s %s\n", g_programName, g_programVersion);
    char		*msgUsage   = Strdup_printf("Usage: %s [OPTION] ...\n", __progname);
    const size_t	 ar_sz	    = ARRAY_SIZE(help_text);

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
create_config_file(const char *path)
{
    FILE		*fp   = NULL;
    const mode_t	 mode = S_IRUSR | S_IWUSR;	/* Read and write for the owner. */
    
    if (file_exists(path)) {
	log_die(EEXIST, "create_config_file: can't create config file  --  it's already existent");
    } else if ((fp = fopen(path, "w")) == NULL) {
	log_die(errno, "create_config_file: fopen error");
    } else {
	struct config_default_values_tag *cdv;
	const size_t ar_sz = ARRAY_SIZE(config_default_values);

	for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	    char *ans = get_answer(cdv->description, cdv->type, cdv->value);
	    
	    if (fprintf(fp, "%s = \"%s\";\n", cdv->setting_name, ans) < 0)
		log_die(0, "create_config_file: failed to write to the file stream");

	    free(ans);
	}

	fclose(fp);
    }

    if (chmod(path, mode) != 0) {
	log_die(errno, "create_config_file: chmod error");
    }
}

static char *
get_answer(const char *desc, enum setting_type type, const char *defaultAnswer)
{
    const size_t	 sz	= 390;
    char		*answer = xcalloc(sz, 1);

    puts(desc);
    printf("Ans [%s]: ", defaultAnswer);
    fflush(stdout);

    if (fgets(answer, sz, stdin) == NULL) {
	log_die(errno, "get_answer: FATAL: fgets fail");
    } else {
	const bool input_too_big = strchr(answer, '\n') == NULL;

	if (input_too_big) log_die(0, "get_answer: FATAL: input too big");
	answer[strcspn(answer, "\n")] = '\0';

	if (Strings_match(answer, "")) {
	    free(answer);
	    answer = xstrdup(defaultAnswer);
	}
    }

    if (!is_setting_ok(answer, type))
	exit(1);
    
    return (answer);
}

static bool
is_setting_ok(const char *value, enum setting_type type)
{
    switch (type) {
    case TYPE_BOOLEAN:
    {
	if (!Strings_match(value, "yes") && !Strings_match(value, "YES") &&
	    !Strings_match(value, "no") && !Strings_match(value, "NO")) {
	    log_warn(0, "is_setting_ok: booleans must be either: yes, YES, no or NO");
	    return false;
	}
	break;
    }
    case TYPE_INTEGER:
    {
	if (!is_numeric(value)) {
	    log_warn(0, "is_setting_ok: integer not all numeric");
	    return false;
	}
	break;
    }
    case TYPE_STRING:
    {
	if (strpbrk(value, " \f\n\r\t\v\"") != NULL) {
	    log_warn(0, "is_setting_ok: illegal characters in string");
	    return false;
	}
	break;
    }
    default:
	assert(false); /* Should not be reached. */
	break;
    }
    
    return true;
}

static void
read_config_file(const char *path)
{
    static bool	 file_read = false;
    FILE	*fp	   = NULL;

    if (file_read) {
	return;
    } else if (!is_regularFile(path)) {
	log_die(0, "read_config_file: either the config file is nonexistent  --  or it isn't a regular file");
    } else if ((fp = fopen(path, "r")) == NULL) {
	log_die(errno, "read_config_file: fopen error");
    } else {
	char		buf[900];
	long int	line_num = 0;
	
	while (BZERO(buf, sizeof buf), fgets(buf, sizeof buf, fp) != NULL) {
	    const char			*ccp	     = &buf[0];
	    const char			 commentChar = '#';
	    char			*line	     = NULL;
	    struct Interpreter_in	 in;

	    adv_while_isspace(&ccp);
	    if (Strings_match(ccp, "") || *ccp == commentChar) {
		line_num++;
		continue;
	    }

	    line = trim(xstrdup(ccp));
	    in.path	      = (char *) path;
	    in.line	      = line;
	    in.validator_func = is_recognized_setting;
	    in.install_func   = install_setting;
	    Interpreter(&in);
	    free_not_null(line);
	}
    }

    if (feof(fp)) {
	fclose(fp);
    } else if (ferror(fp)) {
	log_die(0, "read_config_file: FATAL: fgets returned null and the error indicator is set");
    } else {
	log_die(0, "read_config_file: FATAL: fgets returned null and the reason cannot be determined");
    }

    file_read = true;
}

static void
turn_on_debug_mode(void)
{
    g_debug_mode = true;
}

static void
force_priv_drop(void)
{
    struct passwd *pw;

    log_msg("Dropping root privileges...");
    
    if ((pw = getpwnam(educ_noip_user)) == NULL)
	log_die(0, "getpwnam: no such user %s", educ_noip_user);
    else if (file_exists(educ_noip_dir) && chdir(educ_noip_dir) != 0)
	log_die(errno, "chdir %s", educ_noip_dir);
    else if (setgid(pw->pw_gid)  == -1) log_die(errno, "setgid");
    else if (setegid(pw->pw_gid) == -1) log_die(errno, "setegid");
    else if (setuid(pw->pw_uid)  == -1) log_die(errno, "setuid");
    else if (seteuid(pw->pw_uid) == -1) log_die(errno, "seteuid");
    else return;
}

static bool
is_recognized_setting(const char *setting_name)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);

    for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	if (Strings_match(setting_name, cdv->setting_name))
	    return true;
    }

    return false;
}

static int
install_setting(const char *setting_name, const char *value)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);

    for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	if (Strings_match(setting_name, cdv->setting_name)) {
	    if (cdv->custom_val) {
		return (EBUSY);
	    } else if (!is_setting_ok(value, cdv->type)) {
		return (EINVAL);
	    } else {
		cdv->custom_val = xstrdup(value);
		return (0);
	    }
	}
    }

    return (ENOENT);
}

void
destroy_config_customValues(void)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);

    for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++)
	if (cdv->custom_val) {
	    free(cdv->custom_val);
	    cdv->custom_val = NULL;
	}
}

const char *
setting(const char *setting_name)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);

    for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	if (Strings_match(setting_name, cdv->setting_name))
	    return (cdv->custom_val ? cdv->custom_val : cdv->value);
    }

    return ("");
}

long int
setting_integer_unparse(const struct integer_unparse_context *ctx)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);
    long int val;

    for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	if (Strings_match(ctx->setting_name, cdv->setting_name)) {
	    errno = 0;
	    val = strtol(cdv->custom_val ? cdv->custom_val : cdv->value, NULL, 10);
	    if (errno != 0 || (val < ctx->lo_limit || val > ctx->hi_limit)) break;
	    else return (val);
	}
    }

    return (ctx->fallback_val);
}

static void
start_update_cycle(void)
{
    const size_t ar_sz = ARRAY_SIZE(hostname_array);
    struct integer_unparse_context ctx = {
	.setting_name = "update_interval_seconds",
	.lo_limit     = 600,	/* 10 minutes */
	.hi_limit     = 172800,	/* 2 days */
	.fallback_val = 1800,	/* 30 minutes */
    };
    struct timespec ts = {
	.tv_sec	 = 0,
	.tv_nsec = 0,
    };

    hostname_array_init();
    
    do {
	bool updateRequest_after_30m = false;

	hostname_array_assign();
	
	for (char **ar_p = &hostname_array[0]; ar_p < &hostname_array[ar_sz] && *ar_p && !updateRequest_after_30m; ar_p++) {
	    log_msg("Trying to update %s...", *ar_p);

	    if (!update_host(*ar_p, setting("ip_addr"), &updateRequest_after_30m))
		break; /* Stop updating on the first unsuccessful try. */
	}

	hostname_array_destroy();
	
	ts.tv_sec = updateRequest_after_30m ? 1800 : setting_integer_unparse(&ctx);
	log_debug("Sleeping for %ld seconds...", (long int) ts.tv_sec);
	nanosleep(&ts, NULL);
    } while (true);
}

static void
hostname_array_init(void)
{
    const size_t ar_sz = ARRAY_SIZE(hostname_array);

    /* Initialize all ptrs to NULL. */
    for (char **ar_p = &hostname_array[0]; ar_p < &hostname_array[ar_sz]; ar_p++) *ar_p = NULL;
}

static void
hostname_array_destroy(void)
{
    const size_t ar_sz = ARRAY_SIZE(hostname_array);

    for (char **ar_p = &hostname_array[0]; ar_p < &hostname_array[ar_sz]; ar_p++) {
	free_not_null(*ar_p);
	*ar_p = NULL;
    }
}

static void
hostname_array_assign(void)
{
    char *dump = xstrdup(setting("hostname"));
    const char legal_index[] =
	"abcdefghijklmnopqrstuvwxyz-0123456789.ABCDEFGHIJKLMNOPQRSTUVWXYZ|";

    if (Strings_match(dump, "")) {
	log_die(EINVAL, "hostname_array_assign: no hostnames to update  --  setting empty");
    }

    for (const char *cp = dump; *cp; cp++) {
	if (strchr(legal_index, *cp) == NULL)
	    log_die(0, "hostname_array_assign: invalid chars in setting: first invalid char was '%c'...", *cp);
    }

    for (size_t hosts_assigned = 0;; hosts_assigned++) {
	char *token = strtok(hosts_assigned == 0 ? dump : NULL, "|");

	if (token && hosts_assigned < ARRAY_SIZE(hostname_array))
	    hostname_array[hosts_assigned] = xstrdup(token);
	else {
	    log_debug("hostname_array_assign: a total of %zu hosts were assigned!", hosts_assigned);
	    break;
	}
    }

    free(dump);
}

static bool
update_host(const char *which_host, const char *to_ip, bool *updateRequest_after_30m)
{
    char	*buf = NULL;
    bool	 ok  = true;

    if (net_connect() == -1 || send_update_request(which_host, to_ip) == -1 || store_server_resp_in_buffer(&buf) == -1) {
	ok = false;
	goto err;
    }
    
    switch (server_response(buf)) {
    case CODE_GOOD:
	log_msg("*** DNS hostname update successful! ***");
	break;
    case CODE_NOCHG:
	log_msg("*** IP address is current! ***");
	break;
    case CODE_NOHOST:
	log_die(0, "Hostname supplied does not exist under specified account.");
    case CODE_BADAUTH:
	log_die(0, "Invalid username password combination.");
    case CODE_BADAGENT:
	log_die(0, "Bad agent? I don't think so! But the server is always right. Dying...");
    case CODE_NOTDONATOR:
	log_die(0, "Bad update request. Feature not available.");
    case CODE_ABUSE:
	log_die(0, "Username blocked due to abuse.");
    case CODE_EMERG:
	log_warn(0, "Fatal error on the server side. Will retry update after 30 minutes.");
	*updateRequest_after_30m = true;
	break;
    default:
    case CODE_UNKNOWN:
	log_die(0, "Unknown server response!");
    }

  err:
    if (g_on_air) {
	log_debug("Closing connection to server...");
	close(g_socket);
	g_on_air = false;
    }

    free_not_null(buf);

    return (ok);
}

static int
send_update_request(const char *which_host, const char *to_ip)
{
    extern int b64_encode(uint8_t const *src, size_t srclength, char *target, size_t targsize);
    char	*s, *host, *unp;
    char	 buf[500] = "";
    char	*auth;
    char	*agent;
    bool	 ok = true;

    if (Strings_match(to_ip, "WAN_address")) {
	s = Strdup_printf("GET /nic/update?hostname=%s HTTP/1.0", which_host);
    } else {
	s = Strdup_printf("GET /nic/update?hostname=%s&myip=%s HTTP/1.0", which_host, to_ip);
    }

    host = Strdup_printf("Host: %s", setting("sp_hostname"));
    unp	 = Strdup_printf("%s:%s", setting("username"), setting("password"));
    b64_encode((uint8_t *) unp, strlen(unp), buf, sizeof buf);
    free(unp);
    auth  = Strdup_printf("Authorization: Basic %s", buf);
    agent = Strdup_printf("User-Agent: %s/%s %s", g_programName, g_programVersion, g_maintainerEmail);

    log_debug("Sending http GET request...");
    if (net_send("%s\r\n%s\r\n%s\r\n%s", s, host, auth, agent) != 0) ok = false;
    free(s);
    free(host);
    free(auth);
    free(agent);
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

    return net_recv(*buf, sz);
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
    
    if (buf == NULL || Strings_match(buf, "")) {
	return CODE_UNKNOWN;
    } else {
	char *buf_copy = xstrdup(buf);
	char *buf_ptr;

	while ((buf_ptr = strpbrk(buf_copy, "\r\n")) != NULL)
	    if (*buf_ptr == '\r') {
		*buf_ptr = 'R';
	    } else {
		*buf_ptr = 'N';
	    }

	log_debug("server_response: the buffer looks like this: %s", buf_copy);
	free(buf_copy);
    }

    dump = trim(xstrdup(buf));
    if ((cp = strrchr(dump, '\n')) == NULL) {
	free(dump);
	return CODE_UNKNOWN;
    }
    r = Strtolower(xstrdup(++cp));
    free(dump);
    log_debug("server_response: r = \"%s\"", r);
    
    if (!strncmp(r, "good ", 5)) {
	free(r);
	return CODE_GOOD;
    } else if (!strncmp(r, "nochg ", 6)) {
	free(r);
	return CODE_NOCHG;
    } else {
	for (; ar_p < &responses[ar_sz]; ar_p++)
	    if (Strings_match(ar_p->str, r)) {
		free(r);
		return ar_p->code;
	    }
    }

    free(r);
    return CODE_UNKNOWN;
}
