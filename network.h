#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include <stddef.h>

#include "def.h"

extern int	g_socket;
extern bool	g_on_air;

int net_send    (const char *fmt, ...) PRINTFLIKE(1);
int net_recv    (char *recvbuf, size_t recvbuf_size);
int net_connect (void);

#endif
