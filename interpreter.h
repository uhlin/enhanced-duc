#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <ctype.h> /* isspace */
#include <stdbool.h>

#include "ducdef.h"

enum setting_type {
    TYPE_BOOLEAN,
    TYPE_INTEGER,
    TYPE_STRING
};

typedef bool (*Interpreter_vFunc)(const char *);
typedef int (*Interpreter_instFunc)(const char *, const char *);

struct Interpreter_in {
    char			*path;
    char			*line;
    long int			 line_num;
    Interpreter_vFunc		 validator_func;
    Interpreter_instFunc	 install_func;
};

__BEGIN_DECLS
void Interpreter(const struct Interpreter_in *) /* PTR_ARGS_NONNULL */;
__END_DECLS

static inline void
adv_while_isspace(const char **ptr)
{
    while (isspace(**ptr))
	(*ptr)++;
}

#endif
