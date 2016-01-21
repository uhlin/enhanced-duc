/* Copyright (c) 2016 Markus Uhlin <markus.uhlin@bredband.net>
   All rights reserved.

   Permission to use, copy, modify, and distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
   AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
   PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE. */

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netdb.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "network.h"
#include "settings.h"
#include "wrapper.h"

#define IO_MULTIPLEXING 1
#define SOCKET_CREATION_FAILED -1

int	g_socket = -1;
bool	g_on_air = false;

static struct addrinfo *net_addr_resolve(const char *host, const char *port);

int
net_send(const char *fmt, ...)
{
    extern int my_vasprintf(char **ret, const char *format, va_list ap);
    va_list     ap;
    int         chars_printed;
    char       *buffer;
    const char  message_terminate[] = "\r\n\r\n";
    bool        ok = true;

    va_start(ap, fmt);
    chars_printed = my_vasprintf(&buffer, fmt, ap);
    va_end(ap);

    if (chars_printed < 0) log_die(errno, "net_send: my_vasprintf error");
    buffer = xrealloc(buffer, strlen(buffer) + sizeof message_terminate);
    strcat(buffer, message_terminate);

    if (send(g_socket, buffer, strlen(buffer), 0) == -1) {
	log_warn(errno, "net_send: send error");
	ok = false;
    }

    free(buffer);
    return ok ? 0 : -1;
}

int
net_recv(char *recvbuf, size_t recvbuf_size)
{
#if IO_MULTIPLEXING
    fd_set		readset;
    struct timeval	tv;
    const int		maxfdp1 = g_socket + 1;

    FD_ZERO(&readset);
    FD_SET(g_socket, &readset);

    tv.tv_sec  = 10;
    tv.tv_usec = 0;

    if (select(maxfdp1, &readset, NULL, NULL, &tv) == -1) {
	log_warn(errno, "net_recv: select error");
	return -1;
    } else if (!FD_ISSET(g_socket, &readset)) {
	log_warn(0, "net_recv: no data to receive  --  timed out!");
	return -1;
    } else {
	;
    }
#endif /* IO_MULTIPLEXING */
    switch (recv(g_socket, recvbuf, recvbuf_size, 0)) {
    case -1:
	log_warn(errno, "net_recv: recv error");
	return -1;
    case 0:
	log_warn(0, "net_recv: fatal: connection lost");
	return -1;
    default:
	break;
    }

    return 0;
}

static struct addrinfo *
net_addr_resolve(const char *host, const char *port)
{
    struct addrinfo	 hints;
    struct addrinfo	*res;

    hints.ai_flags     = AI_CANONNAME;
    hints.ai_family    = AF_UNSPEC;
    hints.ai_socktype  = SOCK_STREAM;
    hints.ai_protocol  = 0;
    hints.ai_addrlen   = 0;
    hints.ai_addr      = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next      = NULL;

    if (getaddrinfo(host, port, &hints, &res) != 0) {
	return (NULL);
    }
    
    return (res);
}

int
net_connect(void)
{
    const char		*host = setting("sp_hostname");
    const char		*port = setting("port");
    struct addrinfo	*res, *rp;

    log_debug("Connecting to %s/%s...", host, port);

    if ((res = net_addr_resolve(host, port)) == NULL) {
	log_warn(0, "Unable to get a list of IP addresses. Bogus hostname?");
	return -1;
    } else {
	log_debug("Get a list of IP addresses complete");
    }

    for (rp = res; rp; rp = rp->ai_next)
	if ((g_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == SOCKET_CREATION_FAILED) {
	    continue;
	} else if (connect(g_socket, rp->ai_addr, rp->ai_addrlen) == 0) {
	    log_debug("Connected!");
	    g_on_air = true;
	    break;
	} else {
	    close(g_socket);
	}

    freeaddrinfo(res);
    
    if (!g_on_air) {
	log_warn(0, "Failed to establish a connection");
	return -1;
    }

    return 0;
}
