#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include <stddef.h>

#include "def.h"

typedef enum {
    IP_HAS_CHANGED,
    IP_NO_CHANGE
} ip_chg_t;

typedef int (*NET_SEND_FUNCPTR)(const char *, ...);
typedef int (*NET_RECV_FUNCPTR)(char *, size_t);

extern NET_SEND_FUNCPTR net_send;
extern NET_RECV_FUNCPTR net_recv;

extern int g_socket;

int		net_connect(void);
int		net_recv_plain(char *recvbuf, size_t recvbuf_size);
int		net_send_plain(const char *fmt, ...) PRINTFLIKE(1);
ip_chg_t	net_check_for_ip_change(void);
void		net_deinit(void);
void		net_disconnect(void);
void		net_init(void);

int	net_ssl_recv   (char *recvbuf, size_t recvbuf_size);
int	net_ssl_send   (const char *fmt, ...) PRINTFLIKE(1);
int	net_ssl_start  (void);
void	net_ssl_close  (void);
void	net_ssl_deinit (void);
void	net_ssl_init   (void);

#endif
