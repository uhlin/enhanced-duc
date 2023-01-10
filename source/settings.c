/* Copyright (c) 2016-2023 Markus Uhlin <markus.uhlin@bredband.net>
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

#include "colors.h"
#include "log.h"
#include "settings.h"
#include "various.h"
#include "wrapper.h"

bool g_conf_read = false;

static const char GfxFailure[] = "[\x1b[1;31m*\x1b[0m]";
static const char GfxSuccess[] = "[\x1b[1;32m*\x1b[0m]";

#include "optdesc.h"

static struct config_default_values_tag {
	char *setting_name;
	enum setting_type type;
	char *value;
	char *custom_val;
	const char *description;
} config_default_values[] = {
	{ "username",
	  TYPE_STRING,
	  "ChangeMe",
	  NULL, USERNAME_DESC },
	{ "password",
	  TYPE_STRING,
	  "ChangeMe",
	  NULL, PASSWORD_DESC },
	{ "hostname",
	  TYPE_STRING,
	  "host1.domain.com|host2.domain.com",
	  NULL, HOSTNAME_DESC },
	{ "ip_addr",
	  TYPE_STRING,
	  "WAN_address",
	  NULL, IP_ADDR_DESC },
	{ "sp_hostname",
	  TYPE_STRING,
	  "dynupdate.noip.com",
	  NULL, SP_HOSTNAME_DESC },
	{ "port",
	  TYPE_INTEGER,
	  "80",
	  NULL, PORT_DESC },
	{ "update_interval_seconds",
	  TYPE_INTEGER,
	  "1800",
	  NULL, UPDATE_INTERVAL_SECONDS_DESC },
	{ "primary_ip_lookup_srv",
	  TYPE_STRING,
	  "ip1.dynupdate.no-ip.com",
	  NULL, PRIMARY_IP_LOOKUP_SRV_DESC },
	{ "backup_ip_lookup_srv",
	  TYPE_STRING,
	  "ip2.dynupdate.no-ip.com",
	  NULL, BACKUP_IP_LOOKUP_SRV_DESC },
	{ "force_update",
	  TYPE_BOOLEAN,
	  "YES",
	  NULL, FORCE_UPDATE_DESC },
};

static const size_t CDV_AR_SZ = nitems(config_default_values);

#define FOREACH_CDV()\
    for (struct config_default_values_tag *cdv = &config_default_values[0];\
	 cdv < &config_default_values[CDV_AR_SZ];\
	 cdv++)

/**
 * Get a setting of type boolean. In either way: if a setting isn't
 * found at all, or the setting found isn't of type boolean, it
 * returns the fallback value.
 *
 * @param setting_name	Setting name
 * @param fallback_val	Fallback value
 * @return true or false
 */
bool
setting_bool(const char *setting_name, const bool fallback_val)
{
	if (setting_name == NULL)
		return fallback_val;
	FOREACH_CDV() {
		if (strings_match(setting_name, cdv->setting_name)) {
			const char *value =
			    (cdv->custom_val ? cdv->custom_val : cdv->value);

			if (cdv->type != TYPE_BOOLEAN) {
				log_warn(0, "setting_bool: %s: "
				    "setting not a boolean!", setting_name);
				break;
			} else if (strings_match(value, "yes") ||
				   strings_match(value, "YES")) {
				return true;
			} else if (strings_match(value, "no") ||
				   strings_match(value, "NO")) {
				return false;
			} else {
				log_warn(0, "setting_bool: %s: "
				    "setting has an invalid value!",
				    setting_name);
				break;
			}
		}
	}
	return fallback_val;
}

static bool
is_setting_ok(const char *value, enum setting_type type)
{
	if (value == NULL)
		return false;

	switch (type) {
	case TYPE_BOOLEAN: {
		if (!strings_match(value, "yes") &&
		    !strings_match(value, "YES") &&
		    !strings_match(value, "no") &&
		    !strings_match(value, "NO")) {
			log_warn(0, "%s is_setting_ok: "
			    "booleans must be either: yes, YES, no or NO",
			    GfxFailure);
			return false;
		}
		break;
	}
	case TYPE_INTEGER: {
		if (!is_numeric(value)) {
			log_warn(0, "%s is_setting_ok: integer not all numeric",
			    GfxFailure);
			return false;
		}
		break;
	}
	case TYPE_STRING: {
		if (strpbrk(value, " \f\n\r\t\v\"") != NULL) {
			log_warn(0, "%s is_setting_ok: "
			    "illegal characters in string", GfxFailure);
			return false;
		}
		break;
	}
	default:
		fatal(0, "is_setting_ok: statement reached unexpectedly");
		break;
	}

	return true;
}

/**
 * Get answer based on setting description and setting type. The
 * storage of the return is dynamically allocated and must be freed
 * after use.
 *
 * @param desc		Setting description
 * @param type		Setting type
 * @param defaultAnswer	Value used on empty input
 * @return The answer (or NULL on error)
 */
char *
get_answer(const char *desc, enum setting_type type, const char *defaultAnswer)
{
	const size_t sz = 390;
	char *answer = xcalloc(sz, 1);

	log_assert_arg_nonnull("get_answer", "desc", desc);
	log_assert_arg_nonnull("get_answer", "defaultAnswer", defaultAnswer);

	(void) fputs(MAGENTA, stdout);
	(void) puts(desc);
	(void) fputs(NORMAL, stdout);

	(void) printf("Ans [%s]: ", defaultAnswer);
	(void) fflush(stdout);

	if (!strncmp(desc, "Your password.", 14))
		toggle_echo(OFF);
	const bool fgets_fail = fgets(answer, sz, stdin) == NULL;
	const int errno_save = errno;
	if (!strncmp(desc, "Your password.", 14)) {
		toggle_echo(ON);
		putchar('\n');
	}

	if (fgets_fail)
		fatal(errno_save, "get_answer: fatal: fgets fail");

	const bool input_too_big = strchr(answer, '\n') == NULL;
	int c = EOF;

	if (input_too_big) {
		fprintf(stderr, "%s get_answer: input too big!\n", GfxFailure);
		while (c = getchar(), c != '\n' && c != EOF)
			/* discard */;
		free(answer);
		return NULL;
	}

	/* trim newline */
	answer[strcspn(answer, "\n")] = '\0';

	if (strings_match(answer, "")) {
		free(answer);
		answer = xstrdup(defaultAnswer);
	}
	if (!is_setting_ok(answer, type)) {
		free(answer);
		return NULL;
	}
	return answer;
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
	if (setting_name == NULL) {
		return ("");
	}

	FOREACH_CDV() {
		if (strings_match(setting_name, cdv->setting_name))
			return (cdv->custom_val ? cdv->custom_val : cdv->value);
	}

	return ("");
}

/**
 * Get a setting of type integer. The context structure specifies the
 * rules.
 *
 * @param ctx Context structure
 * @return The result of the conversion
 */
long int
setting_integer(const struct integer_context *ctx)
{
	long int val = 0;

	log_assert_arg_nonnull("setting_integer", "ctx", ctx);

	FOREACH_CDV() {
		if (strings_match(ctx->setting_name, cdv->setting_name)) {
			const char *str =
			    (cdv->custom_val ? cdv->custom_val : cdv->value);
			errno = 0;
			val = strtol(str, NULL, 10);

			if (errno != 0 ||
			    (val < ctx->lo_limit || val > ctx->hi_limit)) {
				log_warn(ERANGE, "warning: setting %s "
				    "out of range %ld-%ld: "
				    "fallback value is %ld", ctx->setting_name,
				    ctx->lo_limit, ctx->hi_limit,
				    ctx->fallback_val);
				break;
			} else {
				return val;
			}
		}
	}

	return (ctx->fallback_val);
}

static bool
is_ip_addr_ok(char **reason)
{
	const char	*ip = setting("ip_addr");
	unsigned char	 buf[sizeof(struct in_addr)];

	if (strings_match(ip, "")) {
		*reason = "empty setting";
		return false;
	} else if (strings_match(ip, "WAN_address")) {
		*reason = "";
		return true;
	} else if (inet_pton(AF_INET, ip, buf) == 0) {
		*reason = "bogus ipv4 address";
		return false;
	}

	*reason = "";
	return true;
}

static bool
is_hostname_ok(const char *host, char **reason)
{
	const char host_chars[] =
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "abcdefghijklmnopqrstuvwxyz"
	    "0123456789-.:";
	const size_t host_maxlen = 255;

	if (strings_match(host, "")) {
		*reason = "empty setting";
		return false;
	} else if (strlen(host) > host_maxlen) {
		*reason = "name too long";
		return false;
	} else {
		for (const char *cp = host; *cp; cp++) {
			if (strchr(host_chars, *cp) == NULL) {
				*reason = "invalid chars found!";
				return false;
			}
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
	return false;
}

/**
 * Check some settings strictly. That is validate that certain
 * settings are OK.
 */
void
check_some_settings_strictly(void)
{
	char *reason = "";
	const char *password = setting("password");
	const char *username = setting("username");
	const size_t password_maxlen = 120;
	const size_t username_maxlen = 50;

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
	FILE *fp = NULL;
	const mode_t mode = S_IRUSR | S_IWUSR; // Read and write for the owner.

	log_assert_arg_nonnull("create_config_file", "path", path);

	if (file_exists(path)) {
		fatal(EEXIST, "%s create_config_file: can't create config file"
		    "  --  it's already existent", GfxFailure);
	} else if ((fp = fopen(path, "w")) == NULL) {
		fatal(errno, "%s create_config_file: fopen", GfxFailure);
	} else {
		FOREACH_CDV() {
			char *ans = NULL;
			int ret = -1;

			while ((ans = get_answer(cdv->description,
						 cdv->type,
						 cdv->value)) == NULL)
				/* continue */;
			ret = fprintf(fp, "%s = \"%s\";\n",
			    cdv->setting_name, ans);
			if (ret < 0) {
				fatal(0, "%s create_config_file: failed to "
				    "write to the file stream", GfxFailure);
			}
			free(ans);
		}

		(void) fclose(fp);
	}

	if (chmod(path, mode) != 0)
		fatal(errno, "%s create_config_file: chmod", GfxFailure);

	printf("%s %s successfully written!\n", GfxSuccess, path);
}

/**
 * Free dynamically allocated memory. Settings that are customized are
 * read into the memory. This function destroys them.
 */
void
destroy_config_custom_values(void)
{
	FOREACH_CDV() {
		if (cdv->custom_val) {
			free(cdv->custom_val);
			cdv->custom_val = NULL;
		}
	}
}

static bool
is_recognized_setting(const char *setting_name)
{
	if (setting_name == NULL)
		return false;
	FOREACH_CDV() {
		if (strings_match(setting_name, cdv->setting_name))
			return true;
	}
	return false;
}

static int
install_setting(const char *setting_name, const char *value)
{
	if (setting_name == NULL || value == NULL)
		return EINVAL;
	FOREACH_CDV() {
		if (strings_match(setting_name, cdv->setting_name)) {
			if (cdv->custom_val) {
				return EBUSY;
			} else if (!is_setting_ok(value, cdv->type)) {
				return EINVAL;
			} else {
				cdv->custom_val = xstrdup(value);
				return 0;
			}
		}
	}
	return ENOENT;
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
		fatal(0, "read_config_file: either the config file is "
		    "nonexistent  --  or it isn't a regular file");
	} else if ((fp = fopen(path, "r")) == NULL) {
		fatal(errno, "read_config_file: fopen");
	}

	Interpreter_processAllLines(fp, path, is_recognized_setting,
	    install_setting);

	if (feof(fp)) {
		fclose(fp);
	} else if (ferror(fp)) {
		fatal(0, "read_config_file: fatal: fgets returned null and "
		    "the error indicator is set");
	} else {
		fatal(0, "read_config_file: fatal: fgets returned null and "
		    "the reason cannot be determined");
	}

	g_conf_read = true;
}
