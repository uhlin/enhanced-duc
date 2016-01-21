#ifndef SETTINGS_H
#define SETTINGS_H

#include "interpreter.h"

struct integer_unparse_context {
    char	*setting_name;
    long int	 lo_limit;
    long int	 hi_limit;
    long int	 fallback_val;
};

void		 create_config_file          (const char *path);
char		*get_answer                  (const char *desc, enum setting_type, const char *defaultAnswer);
void		 read_config_file            (const char *path);
void		 destroy_config_customValues (void);
const char	*setting                     (const char *setting_name);
long int	 setting_integer_unparse     (const struct integer_unparse_context *);

#endif
