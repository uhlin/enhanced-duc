#ifndef LOG_H
#define LOG_H

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>

#include "ducdef.h"

typedef enum {
	REDIR_OK,
	REDIR_STDERR_FAIL,
	REDIR_STDIN_FAIL,
	REDIR_STDOUT_FAIL
} redir_res_t;

__DUC_BEGIN_DECLS
extern bool	 g_log_to_syslog;
extern bool	 g_debug_mode;

/*lint -sem(fatal, r_no) */

redir_res_t	 redirect_standard_streams(void);
__dead void	 fatal(int, const char *, ...) PRINTFLIKE(2);

void	 log_debug(const char *, ...) PRINTFLIKE(1);
void	 log_init(void);
void	 log_msg(const char *, ...) PRINTFLIKE(1);
void	 log_warn(int, const char *, ...) PRINTFLIKE(2);
__DUC_END_DECLS

static inline void
log_assert_arg_nonnull(const char *in_func, const char *arg_name,
		       const void *arg)
{
	if (arg == NULL) {
		fatal(EINVAL, "In %s: Argument \"%s\" is null  --  "
		    "assertion failed!",
		    (in_func ? in_func : "(no in_func)"),
		    (arg_name ? arg_name : "(no arg_name)"));
	}
}

#endif
