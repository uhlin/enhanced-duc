#ifndef LOG_H
#define LOG_H

#include <stdbool.h>

#include "def.h"

extern bool g_log_to_syslog;
extern bool g_debug_mode;

typedef enum {
    REDIR_OK,
    REDIR_STDERR_FAIL,
    REDIR_STDIN_FAIL,
    REDIR_STDOUT_FAIL
} redir_res_t;

redir_res_t redirect_standard_streams (void);
void        log_init                  (void);
void        log_die                   (int code, const char *fmt, ...) PRINTFLIKE(2) NORETURN;
void        log_warn                  (int code, const char *fmt, ...) PRINTFLIKE(2);
void        log_msg                   (const char *fmt, ...) PRINTFLIKE(1);
void        log_debug                 (const char *fmt, ...) PRINTFLIKE(1);

#endif
