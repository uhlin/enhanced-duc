/* Copyright (c) 2016-2023 Markus Uhlin <markus.uhlin@bredband.net>
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
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "main.h"
#include "network.h"
#include "settings.h"
#include "various.h"
#include "wrapper.h"

NET_SEND_FUNCPTR	net_send = net_send_plain;
NET_RECV_FUNCPTR	net_recv = net_recv_plain;

int g_socket = -1;

/*lint -sem(net_addr_resolve, r_null) */
static struct addrinfo *
net_addr_resolve(const char *host, const char *port)
{
	struct addrinfo *res;
	struct addrinfo hints = { 0 };

	if (host == NULL || port == NULL)
		return NULL;

	hints.ai_flags     = AI_CANONNAME;
	hints.ai_family    = AF_UNSPEC;
	hints.ai_socktype  = SOCK_STREAM;
	hints.ai_protocol  = 0;
	hints.ai_addrlen   = 0;
	hints.ai_addr      = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next      = NULL;

	if (getaddrinfo(host, port, &hints, &res) != 0)
		return NULL;
	return res;
}

static inline bool
ssl_is_enabled(void)
{
	return strcmp(setting("port"), "443") == 0;
}

/**
 * Connect to the service provider with or without TLS/SSL depending
 * on the port number.
 *
 * @return 0 on success, and -1 on failure
 */
int
net_connect(void)
{
	bool			 connected = false;
	const char		*host = setting("sp_hostname");
	const char		*port = setting("port");
	struct addrinfo		*res, *rp;

	log_debug("connecting to %s (%s)...", host, port);

	if ((res = net_addr_resolve(host, port)) == NULL) {
		log_warn(0, "unable to get a list of ip addresses. "
		    "bogus hostname?");
		return -1;
	} else {
		log_debug("get a list of ip addresses complete");
	}

	for (rp = res; rp; rp = rp->ai_next) {
		g_socket =
		    socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if (g_socket == SOCKET_CREATION_FAILED) {
			continue;
		} else if (connect(g_socket, rp->ai_addr, rp->ai_addrlen) == 0) {
			log_debug("connected!");
			connected = true;
			break;
		} else {
			(void) close(g_socket);
		}
	}

	freeaddrinfo(res);

	if (!connected || (ssl_is_enabled() && net_ssl_begin() == -1)) {
		log_warn(0, "failed to establish a connection");
		return -1;
	}
	if (ssl_is_enabled() &&
	    net_ssl_check_hostname(host, 0) == HOSTNAME_MISMATCH) {
		log_warn(0, "net_ssl_check_hostname: warning: "
		    "hostname checking failed: "
		    "hostname specified by 'sp_hostname' mismatch: "
		    "failed to establish a connection");
		return -1;
	}
	return 0;
}

/**
 * Network disconnect
 */
void
net_disconnect(void)
{
	if (ssl_is_enabled())
		net_ssl_end();
	if (g_socket != -1) {
		(void) close(g_socket);
		g_socket = -1;
	}
}

/**
 * Send a message on a regular socket
 *
 * @param fmt Format control
 * @return 0 on success, and -1 on failure
 */
int
net_send_plain(const char *fmt, ...)
{
	bool		 ok = true;
	char		*buf = NULL;
	size_t		 newSize = 0;
	static const char
			 message_terminate[] = "\r\n\r\n";
	va_list		 ap;

	log_assert_arg_nonnull("net_send_plain", "fmt", fmt);

	va_start(ap, fmt);
	if (errno = 0, my_vasprintf(&buf, fmt, ap) < 0)
		fatal(errno, "net_send_plain: my_vasprintf");
	va_end(ap);

	newSize = strlen(buf) + nitems(message_terminate);
	buf = xrealloc(buf, newSize);

	if (strlcat(buf, message_terminate, newSize) >= newSize)
		fatal(EOVERFLOW, "net_send_plain: strlcat");
	if (errno = 0, send(g_socket, buf, strlen(buf), 0) == -1)
		ok = false;
	if (!ok)
		log_warn(errno, "net_send_plain: send");
	free(buf);
	return (ok ? 0 : -1);
}

/**
 * Receive a message from a regular socket
 *
 * @param recvbuf	Receive buffer
 * @param recvbuf_size	Receive buffer size
 * @return 0 on success, and -1 on failure
 */
int
net_recv_plain(char *recvbuf, size_t recvbuf_size)
{
#if IO_MULTIPLEXING
	const int	 maxfdp1 = g_socket + 1;
	fd_set		 readset;
	struct timeval	 tv;

	tv.tv_sec	= 10;
	tv.tv_usec	= 333;

	FD_ZERO(&readset);
	FD_SET(g_socket, &readset);

	if (select(maxfdp1, &readset, NULL, NULL, &tv) == -1) {
		log_warn(errno, "net_recv_plain: select");
		return -1;
	} else if (!FD_ISSET(g_socket, &readset)) {
		log_warn(0, "net_recv_plain: no data to receive  --  "
		    "timed out!");
		return -1;
	}
#endif /* IO_MULTIPLEXING */

	log_assert_arg_nonnull("net_recv_plain", "recvbuf", recvbuf);

	switch (recv(g_socket, recvbuf, recvbuf_size, 0)) {
	case -1:
		log_warn(errno, "net_recv_plain: recv");
		return -1;
	case 0:
		log_warn(0, "net_recv_plain: fatal: connection lost");
		return -1;
	default:
		break;
	}
	return 0;
}

/**
 * Check for IP change. The function may return IP_HAS_CHANGED even
 * though the IP hasn't changed, but that is mainly for error
 * conditions to enforce an update to occur. In the same manner: it
 * might return IP_NO_CHANGE if, for example, the received data
 * contains a bogus ipv4 address.
 *
 * @return IP_HAS_CHANGED or IP_NO_CHANGE
 */
ip_chg_t
net_check_for_ip_change(void)
{
	bool			 address_resolved = false;
	bool			 connected = false;
	char			 buf[1000] = "";
	char			 srv[255] = "";
	const char		*backup_srv = setting("backup_ip_lookup_srv");
	const char		*port = "80";
	const char		*primary_srv = setting("primary_ip_lookup_srv");
	struct addrinfo		*res, *rp;
	unsigned char		 nw_addr[sizeof(struct in_addr)];

	if (setting_bool("force_update", true))
		return IP_HAS_CHANGED;

	if ((res = net_addr_resolve(primary_srv, port)) != NULL) {
		(void) strlcpy(srv, primary_srv, sizeof srv);
		address_resolved = true;
		goto done;
	}
	if ((res = net_addr_resolve(backup_srv, port)) != NULL) {
		(void) strlcpy(srv, backup_srv, sizeof srv);
		address_resolved = true;
	}

  done:
	if (!address_resolved)
		return IP_HAS_CHANGED; /* force update */

	for (rp = res; rp; rp = rp->ai_next) {
		g_socket =
		    socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if (g_socket == SOCKET_CREATION_FAILED) {
			continue;
		} else if (connect(g_socket, rp->ai_addr, rp->ai_addrlen) == 0) {
			connected = true;
			break;
		} else {
			(void) close(g_socket);
		}
	}

	freeaddrinfo(res);

	if (!connected) {
		if (g_socket >= 0)
			(void) close(g_socket);
		g_socket = -1;
		return IP_HAS_CHANGED;
	}

	(void) net_send_plain("GET /index.html HTTP/1.0\r\nHost: %s\r\n"
	    "User-Agent: %s/%s %s", srv, g_programName, g_programVersion,
	    g_maintainerEmail);
	(void) net_recv_plain(buf, sizeof buf);
	(void) close(g_socket);
	g_socket = -1;

	const char *cp = strrchr(trim(buf), '\n');

	if (!cp) {
		log_warn(0, "net_check_for_ip_change: warning: "
		    "cannot locate last occurrance of a newline");
		return IP_NO_CHANGE;
	} else if (inet_pton(AF_INET, ++cp, nw_addr) == 0) {
		log_warn(0, "net_check_for_ip_change: warning: "
		    "bogus ipv4 address");
		return IP_NO_CHANGE;
	} else if (strings_match(cp, g_last_ip_addr)) {
		log_debug("not updating (the external ip hasn't changed)");
		return IP_NO_CHANGE;
	}

	(void) strlcpy(g_last_ip_addr, cp, sizeof g_last_ip_addr);
	log_msg("ip has changed to %s", cp);
	return IP_HAS_CHANGED;
}

/**
 * Initialize networking
 */
void
net_init(void)
{
	if (ssl_is_enabled())
		net_ssl_init();
}

/**
 * Deinitialize networking
 */
void
net_deinit(void)
{
	if (ssl_is_enabled())
		net_ssl_deinit();
	if (g_socket != -1) {
		(void) close(g_socket);
		g_socket = -1;
	}
}
