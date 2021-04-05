#ifndef DAEMONIZE_H
#define DAEMONIZE_H

#include "ducdef.h"

#define FORK_FAILED -1
#define FAILED_TO_ACQUIRE_LOCK -1
#define VALUE_CHILD_PROCESS 0

__DUC_BEGIN_DECLS
extern const char	g_lockfile_path[];
extern int		g_lockfile_fd;

void	daemonize(void);
__DUC_END_DECLS

#endif
