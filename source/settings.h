#ifndef SETTINGS_H
#define SETTINGS_H

#include "interpreter.h"

struct integer_context {
	char *setting_name;
	long int lo_limit;
	long int hi_limit;
	long int fallback_val;
};

__DUC_BEGIN_DECLS
extern bool g_conf_read;

bool		 setting_bool(const char *, const bool);
char		*get_answer(const char *, enum setting_type, const char *);
const char	*setting(const char *);
long int	 setting_integer(const struct integer_context *);
void		 check_some_settings_strictly(void);
void		 create_config_file(const char *);
void		 destroy_config_custom_values(void);
void		 read_config_file(const char *);
__DUC_END_DECLS

#endif
