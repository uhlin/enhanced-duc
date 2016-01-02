#ifndef MAIN_H
#define MAIN_H

extern const char g_programName[];
extern const char g_programVersion[];
extern const char g_programAuthor[];
extern const char g_maintainerEmail[];

typedef enum {
    CODE_GOOD,
    CODE_NOCHG,
    CODE_NOHOST,
    CODE_BADAUTH,
    CODE_BADAGENT,
    CODE_NOTDONATOR,
    CODE_ABUSE,
    CODE_EMERG,
    CODE_UNKNOWN
} response_code_t;

struct integer_unparse_context {
    char	*setting_name;
    long int	 lo_limit;
    long int	 hi_limit;
    long int	 fallback_val;
};

void        destroy_config_customValues (void);
const char *setting                     (const char *setting_name);
long int    setting_integer_unparse     (const struct integer_unparse_context *);

#endif
