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

#include <sys/socket.h> /* AF_INET */
#include <sys/stat.h>

#include <arpa/inet.h> /* inet_pton() */

#include <assert.h>
#include <stdio.h>

#include "log.h"
#include "settings.h"
#include "various.h"
#include "wrapper.h"

bool g_conf_read = false;

static const char GfxFailure[] = "[\x1b[1;31m*\x1b[0m]";
static const char GfxSuccess[] = "[\x1b[1;32m*\x1b[0m]";

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
      "Your password. (Will not echo!)" },

    { "hostname", TYPE_STRING, "host1.domain.com|host2.domain.com", NULL,
      "The hostname to be updated. Multiple hosts are separated with a vertical bar." },

    { "ip_addr", TYPE_STRING, "WAN_address", NULL,
      "Associate the hostname(s) with this IP address. If the special value WAN_address\n"
      "is specified: the WAN address from which the update request is sent from is used." },

    { "sp_hostname", TYPE_STRING, "dynupdate.no-ip.com", NULL,
      "Service provider hostname. The http GET request is sent to this hostname." },

    { "port", TYPE_INTEGER, "80", NULL,
      "Connect to sp_hostname + this port. 443 = enable SSL." },

    { "update_interval_seconds", TYPE_INTEGER, "1800", NULL,
      "Update interval specified in seconds. If a value less than 600 is entered, the\n"
      "underlying code will fallback to 1800 to avoid flooding the server with requests." },

    { "primary_ip_lookup_srv", TYPE_STRING, "ip1.dynupdate.no-ip.com", NULL,
      "Server used to determine your external IP." },

    { "backup_ip_lookup_srv", TYPE_STRING, "ip2.dynupdate.no-ip.com", NULL,
      "Backup server for IP lookups." },

    { "force_update", TYPE_BOOLEAN, "YES", NULL,
      "Even if your external IP address hasn't changed between update intervals, or if\n"
      "the program cannot determine your external IP  --  in either way: force update.\n"
      "This setting should be set to YES if ip_addr is not equal to value WAN_address." },
};

/**
 * Unparse a setting of type boolean. In either way: if a setting
 * isn't found at all, or the setting found isn't of type boolean, it
 * returns the fallback value.
 *
 * @param setting_name	Setting name
 * @param fallback_val	Fallback value
 * @return true or false
 */
bool
setting_bool_unparse(const char *setting_name, const bool fallback_val)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);

    if (setting_name == NULL) {
	return (fallback_val);
    }

    for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	if (strings_match(setting_name, cdv->setting_name)) {
	    const char *value = cdv->custom_val ? cdv->custom_val : cdv->value;

	    if (cdv->type != TYPE_BOOLEAN) {
		log_warn(0, "setting_bool_unparse: %s: setting not a boolean!", setting_name);
		break;
	    } else if (strings_match(value, "yes") || strings_match(value, "YES")) {
		return (true);
	    } else if (strings_match(value, "no") || strings_match(value, "NO")) {
		return (false);
	    } else {
		log_warn(0, "setting_bool_unparse: %s: setting has an invalid value!", setting_name);
		break;
	    }
	} /* if */
    } /* for */

    return (fallback_val);
}

static bool
is_setting_ok(const char *value, enum setting_type type)
{
    if (value == NULL) {
	return false;
    }

    switch (type) {
    case TYPE_BOOLEAN:
    {
	if (!strings_match(value, "yes") && !strings_match(value, "YES") &&
	    !strings_match(value, "no") && !strings_match(value, "NO")) {
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

/**
 * Get answer based on setting description and setting type. The
 * storage of the return is dynamically allocated and must be freed
 * fter use.
 *
 * @param desc		Setting description
 * @param type		Setting type
 * @param defaultAnswer	Value used on empty input
 * @return The answer
 */
char *
get_answer(const char *desc, enum setting_type type, const char *defaultAnswer)
{
    const size_t	 sz	= 390;
    char		*answer = xcalloc(sz, 1);

    log_assert_arg_nonnull("get_answer", "desc", desc);
    log_assert_arg_nonnull("get_answer", "defaultAnswer", defaultAnswer);

    puts(desc);
    printf("Ans [%s]: ", defaultAnswer);
    fflush(stdout);

    if (!strncmp(desc, "Your password.", 14))
	toggle_echo(OFF);
    const bool	fgets_fail = fgets(answer, sz, stdin) == NULL;
    const int	errno_save = errno;
    if (!strncmp(desc, "Your password.", 14)) {
	putchar('\n');
	toggle_echo(ON);
    }

    if (fgets_fail) {
	fatal(errno_save, "get_answer: FATAL: fgets fail");
    } else {
	const bool input_too_big = strchr(answer, '\n') == NULL;

	if (input_too_big) fatal(0, "get_answer: FATAL: input too big");
	answer[strcspn(answer, "\n")] = '\0';

	if (strings_match(answer, "")) {
	    free(answer);
	    answer = xstrdup(defaultAnswer);
	}
    }

    if (!is_setting_ok(answer, type))
	exit(1);

    return (answer);
}

/**
 * Lookup a setting. If a null pointer is passed, or if a setting
 * isn't found, it returns an empty string.
 *
 * @param setting_name Setting name
 * @return Setting value
 */
const char *
setting(const char *setting_name)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);

    if (setting_name == NULL) {
	return ("");
    }

    for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	if (strings_match(setting_name, cdv->setting_name))
	    return (cdv->custom_val ? cdv->custom_val : cdv->value);
    }

    return ("");
}

/**
 * Unparse a setting of type integer. The context structure specifies
 * the rules.
 *
 * @param ctx Context structure
 * @return The result of the conversation
 */
long int
setting_integer_unparse(const struct integer_unparse_context *ctx)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);
    long int val;

    log_assert_arg_nonnull("setting_integer_unparse", "ctx", ctx);

    for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	if (strings_match(ctx->setting_name, cdv->setting_name)) {
	    errno = 0;
	    val	  = strtol(cdv->custom_val ? cdv->custom_val : cdv->value, NULL, 10);

	    if (errno != 0 || (val < ctx->lo_limit || val > ctx->hi_limit)) {
		log_warn(ERANGE, "warning: setting %s out of range %ld-%ld: fallback value is %ld",
			 ctx->setting_name, ctx->lo_limit, ctx->hi_limit, ctx->fallback_val);
		break;
	    } else {
		return (val);
	    }
	}
    }

    return (ctx->fallback_val);
}

static bool
is_ip_addr_ok(char **reason)
{
    const char		*ip = setting("ip_addr");
    unsigned char	 buf[sizeof (struct in_addr)];

    if (strings_match(ip, "")) {
	*reason = "empty setting";
	return false;
    } else if (strings_match(ip, "WAN_address")) {
	*reason = "";
	return true;
    } else if (inet_pton(AF_INET, ip, buf) == 0) {
	*reason = "bogus ipv4 address";
	return false;
    } else {
	*reason = "";
	return true;
    }

    /*NOTREACHED*/ *reason = "";
    /*NOTREACHED*/ return true;
}

static bool
is_hostname_ok(const char *host, char **reason)
{
    const size_t host_maxlen = 253;
    const char host_chars[] =
	"abcdefghijklmnopqrstuvwxyz.0123456789-ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    if (strings_match(host, "")) {
	*reason = "empty setting";
	return false;
    } else if (strlen(host) > host_maxlen) {
	*reason = "name too long";
	return false;
    } else {
	for (const char *cp = host; *cp; cp++)
	    if (strchr(host_chars, *cp) == NULL) {
		*reason = "invalid chars found!";
		return false;
	    }
    }

    *reason = "";
    return true;
}

static bool
is_port_ok(void)
{
    const char *port = setting("port");

    if (strings_match(port, "80"))
	return true;
    else if (strings_match(port, "443"))
	return true;
    else if (strings_match(port, "8245"))
	return true;
    else
	return false;

    /*NOTREACHED*/ return false;
}

/**
 * Check some settings strictly. That is validate that certain
 * settings are OK.
 */
void
check_some_settings_strictly(void)
{
    const char *username = setting("username");
    const char *password = setting("password");
    const size_t username_maxlen = 50;
    const size_t password_maxlen = 120;
    char *reason = "";

    if (strings_match(username, "") || strings_match(password, ""))
	fatal(0, "error: empty username nor password");
    else if (strlen(username) > username_maxlen)
	fatal(0, "error: username too long. max=%zu", username_maxlen);
    else if (strlen(password) > password_maxlen)
	fatal(0, "error: password too long. max=%zu", password_maxlen);
    else if (!is_ip_addr_ok(&reason))
	fatal(0, "is_ip_addr_ok: error: %s", reason);
    else if (!is_hostname_ok(setting("sp_hostname"), &reason))
	fatal(0, "is_hostname_ok: sp_hostname: %s", reason);
    else if (!is_port_ok())
	fatal(0, "error: bogus port number");
    else if (!is_hostname_ok(setting("primary_ip_lookup_srv"), &reason))
	fatal(0, "is_hostname_ok: primary_ip_lookup_srv: %s", reason);
    else if (!is_hostname_ok(setting("backup_ip_lookup_srv"), &reason))
	fatal(0, "is_hostname_ok: backup_ip_lookup_srv: %s", reason);
    else
	return;
}

/**
 * Creates a configuration file for the DUC by asking the user for
 * input. On empty input (that is: if the user just hits ENTER) a
 * default value is used.
 *
 * @param path Specifies where to create the file including its filename
 * @return Void
 */
void
create_config_file(const char *path)
{
    FILE		*fp   = NULL;
    const mode_t	 mode = S_IRUSR | S_IWUSR;	/* Read and write for the owner. */

    log_assert_arg_nonnull("create_config_file", "path", path);

    if (file_exists(path)) {
	fatal(EEXIST, "%s create_config_file: can't create config file  --  it's already existent", GfxFailure);
    } else if ((fp = fopen(path, "w")) == NULL) {
	fatal(errno, "%s create_config_file: fopen error", GfxFailure);
    } else {
	struct config_default_values_tag *cdv;
	const size_t ar_sz = ARRAY_SIZE(config_default_values);

	for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	    char *ans = get_answer(cdv->description, cdv->type, cdv->value);

	    if (fprintf(fp, "%s = \"%s\";\n", cdv->setting_name, ans) < 0)
		fatal(0, "%s create_config_file: failed to write to the file stream", GfxFailure);

	    free(ans);
	}

	fclose(fp);
    }

    if (chmod(path, mode) != 0) {
	fatal(errno, "%s create_config_file: chmod error", GfxFailure);
    }

    printf("%s %s successfully written!\n", GfxSuccess, path);
}

/**
 * Free dynamically allocated memory. Settings that are customized are
 * read into the memory. This function destroys them.
 */
void
destroy_config_custom_values(void)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);

    for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++)
	if (cdv->custom_val) {
	    free(cdv->custom_val);
	    cdv->custom_val = NULL;
	}
}

static bool
is_recognized_setting(const char *setting_name)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);

    if (setting_name == NULL)
	return false;

    for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	if (strings_match(setting_name, cdv->setting_name))
	    return true;
    }

    return false;
}

static int
install_setting(const char *setting_name, const char *value)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);

    if (setting_name == NULL || value == NULL)
	return (EINVAL);

    for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	if (strings_match(setting_name, cdv->setting_name)) {
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

/**
 * Reads a configuration file line by line, and installs settings,
 * until an end of file condition is entercounted.
 *
 * @param path Path to the file
 * @return Void
 */
void
read_config_file(const char *path)
{
    FILE *fp = NULL;

    log_assert_arg_nonnull("read_config_file", "path", path);

    if (g_conf_read) {
	return;
    } else if (!is_regularFile(path)) {
	fatal(0, "read_config_file: either the config file is nonexistent  --  or it isn't a regular file");
    } else if ((fp = fopen(path, "r")) == NULL) {
	fatal(errno, "read_config_file: fopen error");
    } else {
	char		buf[900];
	long int	line_num = 0;

	while (BZERO(buf, sizeof buf), fgets(buf, sizeof buf, fp) != NULL) {
	    const char			*ccp	     = &buf[0];
	    const char			 commentChar = '#';
	    char			*line	     = NULL;
	    struct Interpreter_in	 in;

	    adv_while_isspace(&ccp);
	    if (strings_match(ccp, "") || *ccp == commentChar) {
		line_num++;
		continue;
	    }

	    line = trim(xstrdup(ccp));
	    in.path	      = (char *) path;
	    in.line	      = line;
	    in.line_num	      = ++line_num;
	    in.validator_func = is_recognized_setting;
	    in.install_func   = install_setting;
	    Interpreter(&in);
	    free_not_null(line);
	}
    }

    if (feof(fp)) {
	fclose(fp);
    } else if (ferror(fp)) {
	fatal(0, "read_config_file: FATAL: fgets returned null and the error indicator is set");
    } else {
	fatal(0, "read_config_file: FATAL: fgets returned null and the reason cannot be determined");
    }

    g_conf_read = true;
}
