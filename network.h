#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include <stddef.h>

#include "def.h"

typedef int (*NET_SEND_FUNCPTR)(const char *, ...);
typedef int (*NET_RECV_FUNCPTR)(char *, size_t);

extern NET_SEND_FUNCPTR net_send;
extern NET_RECV_FUNCPTR net_recv;

extern int g_socket;

void	net_init       (void);
void	net_deinit     (void);
void	net_disconnect (void);
int	net_send_plain (const char *fmt, ...) PRINTFLIKE(1);
int	net_recv_plain (char *recvbuf, size_t recvbuf_size);
int	net_connect    (void);

void	net_ssl_init   (void);
void	net_ssl_deinit (void);
void	net_ssl_close  (void);
int	net_ssl_start  (void);
int	net_ssl_send   (const char *fmt, ...) PRINTFLIKE(1);
int	net_ssl_recv   (char *recvbuf, size_t recvbuf_size);

#endif
