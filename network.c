/* Copyright (c) 2016, 2018 Markus Uhlin <markus.uhlin@bredband.net>
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

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <stdarg.h>
#include <unistd.h>

#include "log.h"
#include "main.h"
#include "network.h"
#include "settings.h"
#include "various.h"
#include "wrapper.h"

#define IO_MULTIPLEXING 1
#define SOCKET_CREATION_FAILED -1

NET_SEND_FUNCPTR net_send = net_send_plain;
NET_RECV_FUNCPTR net_recv = net_recv_plain;

int g_socket = -1;

static struct addrinfo	*net_addr_resolve (const char *host, const char *port);
static inline bool	 is_ssl_enabled   (void);

/**
 * @brief	Initialize networking
 * @return	void
 *
 * Initialize networking.
 */
void
net_init(void)
{
    if (is_ssl_enabled())
	net_ssl_init();
}

/**
 * @brief	Deinitialize networking
 * @return	void
 *
 * Deinitialize networking.
 */
void
net_deinit(void)
{
    if (is_ssl_enabled())
	net_ssl_deinit();

    if (g_socket != -1) {
	close(g_socket);
	g_socket = -1;
    }
}

/**
 * @brief	Network disconnect
 * @return	void
 *
 * Network disconnect.
 */
void
net_disconnect(void)
{
    if (is_ssl_enabled())
	net_ssl_close();

    if (g_socket != -1) {
	close(g_socket);
	g_socket = -1;
    }
}

/**
 * @brief Send a message on a regular socket
 * @param fmt Format control
 * @return 0 on success, and -1 on failure
 *
 * Send a message on a regular socket.
 */
int
net_send_plain(const char *fmt, ...)
{
    extern int my_vasprintf(char **ret, const char *format, va_list ap);
    va_list     ap;
    char       *buffer;
    const char  message_terminate[] = "\r\n\r\n";
    bool        ok = true;

    log_assert_arg_nonnull("net_send_plain", "fmt", fmt);

    va_start(ap, fmt);
    if (my_vasprintf(&buffer, fmt, ap) < 0)
	log_die(errno, "net_send_plain: my_vasprintf error");
    va_end(ap);

    size_t newSize = strlen(buffer) + sizeof message_terminate;
    buffer = xrealloc(buffer, newSize);

    if (strlcat(buffer, message_terminate, newSize) >= newSize)
	log_die(EOVERFLOW, "net_send_plain: strlcat error");

    if (send(g_socket, buffer, strlen(buffer), 0) == -1) {
	log_warn(errno, "net_send_plain: send error");
	ok = false;
    }

    free(buffer);
    return ok ? 0 : -1;
}

/**
 * @brief Receive a message from a regular socket
 * @param recvbuf	Receive buffer
 * @param recvbuf_size	Receive buffer size
 * @return 0 on success, and -1 on failure
 *
 * Receive a message from a regular socket.
 */
int
net_recv_plain(char *recvbuf, size_t recvbuf_size)
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
	log_warn(errno, "net_recv_plain: select error");
	return -1;
    } else if (!FD_ISSET(g_socket, &readset)) {
	log_warn(0, "net_recv_plain: no data to receive  --  timed out!");
	return -1;
    } else {
	;
    }
#endif /* IO_MULTIPLEXING */

    log_assert_arg_nonnull("net_recv_plain", "recvbuf", recvbuf);

    switch (recv(g_socket, recvbuf, recvbuf_size, 0)) {
    case -1:
	log_warn(errno, "net_recv_plain: recv error");
	return -1;
    case 0:
	log_warn(0, "net_recv_plain: fatal: connection lost");
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

    if (host == NULL || port == NULL) {
	return (NULL);
    }

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

/**
 * @brief	Connect to the service provider
 * @return	0 on success, and -1 on failure
 *
 * Connect to the service provider with or without TLS/SSL depending
 * on the port number.
 */
int
net_connect(void)
{
    const char		*host	   = setting("sp_hostname");
    const char		*port	   = setting("port");
    struct addrinfo	*res, *rp;
    bool		 connected = false;

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
	    connected = true;
	    break;
	} else {
	    close(g_socket);
	}

    freeaddrinfo(res);

    if (!connected) {
	log_warn(0, "Failed to establish a connection");
	return -1;
    }

    return (is_ssl_enabled() ? net_ssl_start() : 0);
}

static inline bool
is_ssl_enabled(void)
{
    return (strcmp(setting("port"), "443") == 0);
}

/**
 * @brief	Check for IP change
 * @return	IP_HAS_CHANGED or IP_NO_CHANGE
 *
 * Check for IP change. The function may return IP_HAS_CHANGED even
 * though the IP hasn't changed, but that is mainly for error
 * conditions to enforce an update to occur.  In the same manner: it
 * might return IP_NO_CHANGE if, for example, the received data
 * contains a bogus ipv4 address.
 */
ip_chg_t
net_check_for_ip_change(void)
{
    struct addrinfo *res, *rp;
    const char    *primary_srv      = setting("primary_ip_lookup_srv");
    const char    *backup_srv       = setting("backup_ip_lookup_srv");
    const char    *port             = "80";
    char           srv[255]         = "";
    bool           address_resolved = false;
    bool           connected        = false;
    char           buf[1000]        = "";
    unsigned char  nw_addr[sizeof (struct in_addr)];

    if (setting_bool_unparse("force_update", true))
	return (IP_HAS_CHANGED);

    if ((res = net_addr_resolve(primary_srv, port)) != NULL) {
	strlcpy(srv, primary_srv, sizeof srv);
	address_resolved = true;
	goto done;
    }

    if ((res = net_addr_resolve(backup_srv, port)) != NULL) {
	strlcpy(srv, backup_srv, sizeof srv);
	address_resolved = true;
    }

  done:

    if (!address_resolved)
	return (IP_HAS_CHANGED); /* force update */

    for (rp = res; rp; rp = rp->ai_next) {
	if ((g_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == SOCKET_CREATION_FAILED) {
	    continue;
	} else if (connect(g_socket, rp->ai_addr, rp->ai_addrlen) == 0) {
	    connected = true;
	    break;
	} else {
	    close(g_socket);
	}
    }

    freeaddrinfo(res);

    if (!connected) {
	close(g_socket);
	g_socket = -1;
	return (IP_HAS_CHANGED);
    }

    net_send_plain("GET /index.html HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s/%s %s",
		   srv, g_programName, g_programVersion, g_maintainerEmail);
    net_recv_plain(buf, sizeof buf);
    close(g_socket);
    g_socket = -1;
    trim(buf);
    const char *cp = strrchr(buf, '\n');

    if (!cp) {
	log_warn(0, "net_check_for_ip_change: warning: cannot locate last occurrance of a newline");
	return (IP_NO_CHANGE);
    } else if (inet_pton(AF_INET, ++cp, nw_addr) == 0) {
	log_warn(0, "net_check_for_ip_change: warning: bogus ipv4 address");
	return (IP_NO_CHANGE);
    } else if (strings_match(cp, g_last_ip_addr)) {
	log_msg("Not updating  --  the external IP hasn't changed");
	return (IP_NO_CHANGE);
    } else {
	strlcpy(g_last_ip_addr, cp, sizeof g_last_ip_addr);
	log_msg("IP has changed to %s", cp);
	return (IP_HAS_CHANGED);
    }

    /*NOTREACHED*/ assert(false);
    /*NOTREACHED*/ return (IP_NO_CHANGE);
}
