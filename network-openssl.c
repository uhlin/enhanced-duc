/* Copyright (c) 2016-2019 Markus Uhlin <markus.uhlin@bredband.net>
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

#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>

#include <limits.h>
#include <string.h>

#include "log.h"
#include "network.h"
#include "various.h"
#include "wrapper.h"

static SSL_CTX	*ssl_ctx = NULL;
static SSL	*ssl	 = NULL;

static const char cipher_list[] = "HIGH:!aNULL";

/*lint -sem(get_cert, r_null) */
static X509 *
get_cert()
{
#ifndef UNIT_TESTING
    if (!ssl)
	return NULL;

    return SSL_get_peer_certificate(ssl);
#else
/* ------------ */
/* UNIT_TESTING */
/* ------------ */
    FILE *fp = NULL;
    X509 *cert = NULL;

    if ((fp = fopen("noip.crt", "r")) == NULL ||
	(cert = PEM_read_X509(fp, NULL, NULL, NULL)) == NULL) {
	if (fp)
	    fclose(fp);
	return NULL;
    }

    fclose(fp);
    return cert;
#endif
}

chkhost_res_t
net_ssl_check_hostname(const char *host, unsigned int flags)
{
#if HAVE_X509_CHECK_HOST
    X509 *cert = NULL;
    chkhost_res_t ret = HOSTNAME_MISMATCH;

    if ((cert = get_cert()) == NULL || host == NULL) {
	if (cert)
	    X509_free(cert);
	return HOSTNAME_MISMATCH;
    }
    ret = ((X509_check_host(cert, host, 0, flags, NULL) > 0)
	   ? HOSTNAME_MATCH
	   : HOSTNAME_MISMATCH);
    X509_free(cert);
    return ret;
#else
    log_warn(ENOSYS, "net_ssl_check_hostname: warning: "
	"function not implemented (returning HOSTNAME_MATCH anyway...)");
    return HOSTNAME_MATCH;
#endif
}

/**
 * Write bytes to a TLS/SSL connection
 *
 * @param fmt Format control
 * @return 0 on success, and -1 on failure
 */
int
net_ssl_send(const char *fmt, ...)
{
    static const char message_terminate[] =
	"\r\n\r\n";
    char	*buf	 = NULL;
    int		 buflen	 = 0;
    int		 n_sent	 = 0;
    size_t	 newSize = 0;
    va_list	 ap;

    log_assert_arg_nonnull("net_ssl_send", "fmt", fmt);

    va_start(ap, fmt);
    if (my_vasprintf(&buf, fmt, ap) < 0)
	fatal(errno, "net_ssl_send: my_vasprintf");
    va_end(ap);

    newSize = strlen(buf) + ARRAY_SIZE(message_terminate);
    buf = xrealloc(buf, newSize);

    if (strlcat(buf, message_terminate, newSize) >= newSize)
	fatal(EOVERFLOW, "net_ssl_send: strlcat");

    if (strlen(buf) > INT_MAX) {
	free(buf);
	return -1;
    }

    buflen = (int) strlen(buf);
    ERR_clear_error();

    if ((n_sent = SSL_write(ssl, buf, buflen)) > 0) {
	free(buf);
	return 0;
    }

    free(buf);

    switch (SSL_get_error(ssl, n_sent)) {
    case SSL_ERROR_NONE:
	return 0;
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
	log_warn(0, "net_ssl_send: operation did not complete");
	return 0;
    }

    return -1;
}

/**
 * Read bytes from a TLS/SSL connection
 *
 * @param recvbuf	Receive buffer
 * @param recvbuf_size	Receive buffer size
 * @return 0 on success, and -1 on failure
 */
int
net_ssl_recv(char *recvbuf, size_t recvbuf_size)
{
    const int maxfdp1 = g_socket + 1;
    fd_set readset;
    struct timeval tv = {
	.tv_sec  = 10,
	.tv_usec = 0,
    };

    log_assert_arg_nonnull("net_ssl_recv", "recvbuf", recvbuf);

    FD_ZERO(&readset);
    FD_SET(g_socket, &readset);

    errno = 0;

    if (select(maxfdp1, &readset, NULL, NULL, &tv) == -1) {
	log_warn(errno, "net_ssl_recv: select");
	return -1;
    } else if (!FD_ISSET(g_socket, &readset)) {
	log_warn(0, "net_ssl_recv: no data to receive  --  timed out!");
	return -1;
    }

    int bytes_received = 0;
    ERR_clear_error();

    if ((bytes_received = SSL_read(ssl, recvbuf, recvbuf_size)) > 0)
	return 0;
    switch (SSL_get_error(ssl, bytes_received)) {
    case SSL_ERROR_NONE:
	return 0;
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
	log_warn(0, "net_ssl_recv: want read / want write");
	return 0;
    }

    return -1;
}

/**
 * Initiate the TLS/SSL handshake with an TLS/SSL server
 *
 * @return 0 on success, -1 on error
 */
int
net_ssl_begin(void)
{
    const int VALUE_HANDSHAKE_OK = 1;

    if ((ssl = SSL_new(ssl_ctx)) == NULL)
	fatal(ENOMEM, "net_ssl_begin: unable to create a new ssl object");
    else if (!SSL_set_fd(ssl, g_socket))
	log_warn(0, "net_ssl_begin: "
	    "unable to associate the global socket fd with the ssl object");
    else if (SSL_connect(ssl) != VALUE_HANDSHAKE_OK)
	log_warn(0, "net_ssl_begin: handshake not ok!");
    else
	return (0);

    return (-1);
}

/**
 * Shut down a TLS/SSL connection
 */
void
net_ssl_end(void)
{
    if (ssl) {
	SSL_shutdown(ssl);
	SSL_free(ssl);
	ssl = NULL;
    }
}

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
static void
create_ssl_context_obj()
{
    if ((ssl_ctx = SSL_CTX_new(TLS_client_method())) == NULL) {
	fatal(ENOMEM, "create_ssl_context_obj: "
	    "unable to create a new ssl_ctx object");
    } else {
	SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv2);
	SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv3);
	SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_TLSv1);
	SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_TLSv1_1);
    }
}
#else
/* -------------------------------- */
/* OpenSSL version less than v1.1.0 */
/* -------------------------------- */

static void
create_ssl_context_obj_insecure()
{
    if ((ssl_ctx = SSL_CTX_new(SSLv23_client_method())) == NULL) {
	fatal(ENOMEM, "create_ssl_context_obj_insecure: "
	    "unable to create a new ssl_ctx object");
    } else {
	SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv2);
	SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv3);
    }
}
#endif

static int
verify_callback(int ok, X509_STORE_CTX *ctx)
{
    X509	*cert  = X509_STORE_CTX_get_current_cert(ctx);
    char  issuer[256]  = "";
    char  subject[256] = "";
    const int	 depth = X509_STORE_CTX_get_error_depth(ctx);
    const int	 err   = X509_STORE_CTX_get_error(ctx);

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
 * Initialize the TLS/SSL library
 */
void
net_ssl_init(void)
{
    SSL_load_error_strings();
    SSL_library_init();

    if (RAND_load_file("/dev/urandom", 1024) <= 0)
	log_warn(ENOSYS, "net_ssl_init: error seeding the prng!");

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    create_ssl_context_obj();
#else
#pragma message("Consider updating your TLS/SSL library")
    create_ssl_context_obj_insecure();
#endif

    if (SSL_CTX_set_default_verify_paths(ssl_ctx)) {
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, verify_callback);
	SSL_CTX_set_verify_depth(ssl_ctx, 4);
    } else {
	log_warn(ENOSYS, "net_ssl_init: certificate verification is disabled");
    }

    if (!SSL_CTX_set_cipher_list(ssl_ctx, cipher_list))
	log_warn(EINVAL, "net_ssl_init: bogus cipher list");

    net_send = net_ssl_send;
    net_recv = net_ssl_recv;

    log_msg("TLS/SSL enabled");
}

/**
 * Deinitialize the TLS/SSL library
 */
void
net_ssl_deinit(void)
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
