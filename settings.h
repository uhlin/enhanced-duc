#ifndef SETTINGS_H
#define SETTINGS_H

#include "interpreter.h"

struct integer_unparse_context {
    char	*setting_name;
    long int	 lo_limit;
    long int	 hi_limit;
    long int	 fallback_val;
};

extern bool g_conf_read;

bool		 setting_bool(const char *setting_name,
		     const bool fallback_val);
char		*get_answer(const char *desc, enum setting_type,
		     const char *defaultAnswer);
const char	*setting(const char *setting_name);
long int	 setting_integer_unparse(const struct integer_unparse_context *);
void		 check_some_settings_strictly(void);
void		 create_config_file(const char *path);
void		 destroy_config_custom_values(void);
void		 read_config_file(const char *path);

#endif
