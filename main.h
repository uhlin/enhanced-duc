#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>

#define DUC_PATH_MAX			500	/* Max bytes in a pathname. */
#define DUC_PERMITTED_HOSTS_LIMIT	10	/* Qty of hostnames allowed. */
#define UID_SUPER_USER			0

extern const char g_programName[];
extern const char g_programVersion[];
extern const char g_programAuthor[];
extern const char g_maintainerEmail[];

extern char g_last_ip_addr[100];

struct program_options {
    bool want_usage;
    bool want_create_config_file;
    bool want_debug;
    bool want_update_once;
    bool want_daemon;
};

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

#if __OpenBSD__
int pledge(const char *, const char **);
#endif

#endif
