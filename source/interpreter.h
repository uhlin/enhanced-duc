#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <ctype.h> /* isspace */
#include <stdbool.h>
#include <stdio.h> /* FILE */

#include "ducdef.h"

/*
 * Set to 0 to turn off this feature.
 */
#define IGNORE_UNRECOGNIZED_IDENTIFIERS 1

#define MAXLINE 3200

enum setting_type {
	TYPE_BOOLEAN,
	TYPE_INTEGER,
	TYPE_STRING
};

typedef bool (*Interpreter_vFunc)(const char *);
typedef int (*Interpreter_instFunc)(const char *, const char *);

struct Interpreter_in {
	char *path;
	char *line;
	long int line_num;
	Interpreter_vFunc validator_func;
	Interpreter_instFunc install_func;
};

__DUC_BEGIN_DECLS
extern const char g_fgets_nullret_err1[];
extern const char g_fgets_nullret_err2[];

void	Interpreter(const struct Interpreter_in *);
void	Interpreter_processAllLines(FILE *, const char *, Interpreter_vFunc,
	    Interpreter_instFunc);
__DUC_END_DECLS

static inline void
adv_while_isspace(const char **ptr)
{
	while (isspace(**ptr))
		(*ptr)++;
}

#endif
