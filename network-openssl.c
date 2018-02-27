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

#include <openssl/opensslv.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include "log.h"
#include "network.h"
#include "various.h"
#include "wrapper.h"

static SSL_CTX	*ssl_ctx = NULL;
static SSL	*ssl	 = NULL;

static const char cipher_list[] = "HIGH:@STRENGTH";

static int
verify_callback(int ok, X509_STORE_CTX *ctx)
{
    X509	*cert  = X509_STORE_CTX_get_current_cert(ctx);
    const int	 depth = X509_STORE_CTX_get_error_depth(ctx);
    const int	 err   = X509_STORE_CTX_get_error(ctx);
    char  issuer[256]  = "";
    char  subject[256] = "";

    X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof issuer);
    X509_NAME_oneline(X509_get_subject_name(cert), subject, sizeof subject);

    if (!ok) {
	log_warn(0, "Error with certificate at depth: %d", depth);
	log_warn(0, "  issuer  = %s", issuer);
	log_warn(0, "  subject = %s", subject);
	log_warn(0, "Reason: %s", X509_verify_cert_error_string(err));
    } else {
	log_debug("Cert verification OK!");
	log_debug("  issuer  = %s", issuer);
	log_debug("  subject = %s", subject);
    }

    return (ok);
}

/**
 * @brief	Initialize the TLS/SSL library
 * @return	void
 *
 * Initialize the TLS/SSL library.
 */
void
net_ssl_init()
{
    SSL_load_error_strings();
    SSL_library_init();

    if (RAND_load_file("/dev/urandom", 1024) <= 0) {
	log_warn(ENOSYS, "net_ssl_init: Error seeding the PRNG! LibreSSL?");
    }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    if (( ssl_ctx = SSL_CTX_new(TLS_client_method()) ) == NULL) {
	log_die(ENOMEM, "net_ssl_init: Unable to create a new SSL_CTX object");
    } else {
	SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			    SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
    }
#else
    if (( ssl_ctx = SSL_CTX_new(SSLv23_client_method()) ) == NULL) {
	log_die(ENOMEM, "net_ssl_init: Unable to create a new SSL_CTX object");
    } else {
	SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    }
#endif

    if (SSL_CTX_set_default_verify_paths(ssl_ctx)) {
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, verify_callback);
	SSL_CTX_set_verify_depth(ssl_ctx, 4);
    } else {
	log_warn(ENOSYS, "net_ssl_init: Certificate verification is disabled");
    }

    if (!SSL_CTX_set_cipher_list(ssl_ctx, cipher_list))
	log_warn(EINVAL, "net_ssl_init: Bogus cipher list");

    net_send = net_ssl_send;
    net_recv = net_ssl_recv;

    log_msg("SSL enabled");
}

/**
 * @brief	Deinitialize the TLS/SSL library
 * @return	void
 *
 * Deinitialize the TLS/SSL library.
 */
void
net_ssl_deinit()
{
    if (ssl) {
	SSL_shutdown(ssl);
	SSL_free(ssl);
	ssl = NULL;
    }
    if (ssl_ctx) {
	SSL_CTX_free(ssl_ctx);
	ssl_ctx = NULL;
    }
}

/**
 * @brief	Shut down a TLS/SSL connection
 * @return	void
 *
 * Shut down a TLS/SSL connection.
 */
void
net_ssl_close(void)
{
    if (ssl) {
	SSL_shutdown(ssl);
	SSL_free(ssl);
	ssl = NULL;
    }
}

/**
 * @brief	Initiate the TLS/SSL handshake with an TLS/SSL server
 * @return	0 on success, -1 on error
 *
 * Initiate the TLS/SSL handshake with an TLS/SSL server.
 */
int
net_ssl_start(void)
{
    const int VALUE_HANDSHAKE_OK = 1;
    
    if ((ssl = SSL_new(ssl_ctx)) == NULL)
	log_die(ENOMEM, "net_ssl_start: Unable to create a new SSL object");
    else if (!SSL_set_fd(ssl, g_socket))
	log_warn(0, "net_ssl_start: Unable to associate the global socket fd with the SSL object");
    else if (SSL_connect(ssl) != VALUE_HANDSHAKE_OK)
	log_warn(0, "net_ssl_start: Handshake NOT ok!");
    else
	return (0);
    
    return (-1);
}

/**
 * @brief Write bytes to a TLS/SSL connection
 * @param fmt Format control
 * @return 0 on success, and -1 on failure
 *
 * Write bytes to a TLS/SSL connection.
 */
int
net_ssl_send(const char *fmt, ...)
{
    extern int my_vasprintf(char **ret, const char *format, va_list ap);
    va_list     ap;
    char       *buffer;
    const char  message_terminate[] = "\r\n\r\n";
    bool        ok = true;

    log_assert_arg_nonnull("net_ssl_send", "fmt", fmt);

    va_start(ap, fmt);
    if (my_vasprintf(&buffer, fmt, ap) < 0)
	log_die(errno, "net_ssl_send: my_vasprintf error");
    va_end(ap);

    size_t newSize = strlen(buffer) + sizeof message_terminate;
    buffer = xrealloc(buffer, newSize);

    if (strlcat(buffer, message_terminate, newSize) >= newSize)
	log_die(EOVERFLOW, "net_ssl_send: strlcat error");

    for (int total_written = 0, ret = 0; total_written < strlen(buffer); (void) 0)
	if ((ret = SSL_write(ssl, &buffer[total_written], strlen(buffer) - total_written)) <= 0) {
	    ok = false;
	    break;
	} else {
	    total_written += ret;
	}

    free(buffer);
    return ok ? 0 : -1;
}

/**
 * @brief Read bytes from a TLS/SSL connection
 * @param recvbuf	Receive buffer
 * @param recvbuf_size	Receive buffer size
 * @return 0 on success, and -1 on failure
 *
 * Read bytes from a TLS/SSL connection.
 */
int
net_ssl_recv(char *recvbuf, size_t recvbuf_size)
{
    fd_set		readset;
    struct timeval	tv;
    const int		maxfdp1 = g_socket + 1;

    log_assert_arg_nonnull("net_ssl_recv", "recvbuf", recvbuf);

    FD_ZERO(&readset);
    FD_SET(g_socket, &readset);

    tv.tv_sec  = 10;
    tv.tv_usec = 0;

    if (select(maxfdp1, &readset, NULL, NULL, &tv) == -1) {
	log_warn(errno, "net_ssl_recv: select error");
	return -1;
    } else if (!FD_ISSET(g_socket, &readset)) {
	log_warn(0, "net_ssl_recv: no data to receive  --  timed out!");
	return -1;
    } else {
	int ret = 0, total_read = 0;

	do {
	    while (total_read < recvbuf_size)
		if ((ret = SSL_read(ssl, &recvbuf[total_read], recvbuf_size - total_read)) <= 0) {
		    break;
		} else {
		    total_read += ret;
		}
	} while (ret > 0);

	log_debug("net_ssl_recv: ret=%d and total_read=%d", ret, total_read);
    }

    return 0;
}
