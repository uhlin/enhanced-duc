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
#include <stddef.h>
#include <stdio.h>

#include "log.h"
#include "settings.h"
#include "various.h"
#include "wrapper.h"

bool g_conf_read = false;

static const char GfxFailure[] = "[\x1b[1;32m*\x1b[0m]";
static const char GfxSuccess[] = "[\x1b[1;31m*\x1b[0m]";

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
      "Connect to sp_hostname + this port. 443 = enable SSL." },

    { "update_interval_seconds", TYPE_INTEGER, "1800", NULL,
      "Update interval specified in seconds. If a value less than 600 is entered, the\n"
      "underlying code will fallback to 600 to avoid flooding the server with requests." },
};

static bool	is_setting_ok         (const char *value, enum setting_type);
static bool	is_recognized_setting (const char *setting_name);
static int	install_setting       (const char *setting_name, const char *value);

void
create_config_file(const char *path)
{
    FILE		*fp   = NULL;
    const mode_t	 mode = S_IRUSR | S_IWUSR;	/* Read and write for the owner. */

    log_assert_arg_nonnull("create_config_file", "path", path);

    if (file_exists(path)) {
	log_die(EEXIST, "%s create_config_file: can't create config file  --  it's already existent", GfxFailure);
    } else if ((fp = fopen(path, "w")) == NULL) {
	log_die(errno, "%s create_config_file: fopen error", GfxFailure);
    } else {
	struct config_default_values_tag *cdv;
	const size_t ar_sz = ARRAY_SIZE(config_default_values);

	for (cdv = &config_default_values[0]; cdv < &config_default_values[ar_sz]; cdv++) {
	    char *ans = get_answer(cdv->description, cdv->type, cdv->value);
	    
	    if (fprintf(fp, "%s = \"%s\";\n", cdv->setting_name, ans) < 0)
		log_die(0, "%s create_config_file: failed to write to the file stream", GfxFailure);

	    free(ans);
	}

	fclose(fp);
    }

    if (chmod(path, mode) != 0) {
	log_die(errno, "%s create_config_file: chmod error", GfxFailure);
    }

    printf("%s %s successfully written!\n", GfxSuccess, path);
}

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
    if (value == NULL) {
	return false;
    }

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

void
read_config_file(const char *path)
{
    FILE *fp = NULL;

    log_assert_arg_nonnull("read_config_file", "path", path);

    if (g_conf_read) {
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

    g_conf_read = true;
}

static bool
is_recognized_setting(const char *setting_name)
{
    struct config_default_values_tag *cdv;
    const size_t ar_sz = ARRAY_SIZE(config_default_values);

    if (setting_name == NULL)
	return false;

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

    if (setting_name == NULL || value == NULL)
	return (EINVAL);

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

    if (setting_name == NULL) {
	return ("");
    }

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

    log_assert_arg_nonnull("setting_integer_unparse", "ctx", ctx);

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
