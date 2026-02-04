/*
 *
 * Minimal CyaSSL/OpenSSL/WolfSSL Helper
 * Copyright (C) 2006-2009 Jonathan Zarate
 * Copyright (C) 2010 Fedor Kozhevnikov
 *
 * Licensed under GNU GPL v2 or later
 *
 * Fixes/updates (C) 2018 - 2026 pedro
 * https://freshtomato.org/
 *
 */


#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "mssl.h"
#include "shared.h"

/* refer https://ssl-config.mozilla.org/ w/o DES ciphers */
#ifdef USE_WOLFSSL
 #ifdef USE_WOLFSSLMIN
  #define SERVER_CIPHERS "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA256:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:!DSS"
 #else
  #define SERVER_CIPHERS "ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA256:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:!DSS"
 #endif
#elif USE_OPENSSL
 #define SERVER_CIPHERS "ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA256:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:!DSS"
#elif USE_CYASSL
 #define SERVER_CIPHERS "ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA256:DHE-RSA-AES256-SHA:ECDHE-ECDSA-DES-CBC3-SHA:ECDHE-RSA-DES-CBC3-SHA:EDH-RSA-DES-CBC3-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:DES-CBC3-SHA:!DSS"
#endif

#ifndef USE_LIBCURL
 #ifdef USE_WOLFSSL
  /* use reasonable defaults */
  #define CLIENT_CIPHERS NULL
 #elif USE_OPENSSL
  #if OPENSSL_VERSION_NUMBER >= 0x10100000L
   /* use reasonable defaults */
   #define CLIENT_CIPHERS NULL
  #else
   #define CLIENT_CIPHERS "ALL:!EXPORT:!EXPORT40:!EXPORT56:!aNULL:!LOW:!RC4:@STRENGTH"
  #endif
 #elif USE_CYASSL
  #define CLIENT_CIPHERS "ALL:!EXPORT:!EXPORT40:!EXPORT56:!aNULL:!LOW:!RC4:@STRENGTH"
 #endif
#endif /* !USE_LIBCURL */

/* needed by logmsg() */
#define LOGMSG_DISABLE	DISABLE_SYSLOG_OSM
#define LOGMSG_NVDEBUG	"mssl_debug"


typedef struct {
#ifdef USE_WOLFSSL
	WOLFSSL* ssl;
#else
	SSL* ssl;
#endif
	int sd;
} mssl_cookie_t;

#ifdef USE_WOLFSSL
static WOLFSSL_CTX* ctx;
#else
static SSL_CTX* ctx;
#endif

#ifdef USE_WOLFSSL
static inline void mssl_print_err(WOLFSSL* ssl, int err)
#else
static inline void mssl_print_err(SSL* ssl, int err)
#endif
{
#ifdef USE_WOLFSSL
	char buf[80];
	memset(buf, 0, sizeof(buf));
	wolfSSL_ERR_error_string(err, buf);
	logmsg(LOG_DEBUG, "*** [mssl] error %s", ssl ? buf : "-1");
#elif USE_OPENSSL
	char buf[256];
	memset(buf, 0, sizeof(buf));
	ERR_error_string((unsigned long)err, buf);
	logmsg(LOG_DEBUG, "*** [mssl] error %s", ssl ? buf : "-1");
#elif USE_CYASSL
	logmsg(LOG_DEBUG, "*** [mssl] error %d", ssl ? SSL_get_error(ssl, 0) : -1);
#endif
}

static inline void mssl_cleanup(int err)
{
	if (err)
		mssl_print_err(NULL, 0);

#ifdef USE_WOLFSSL
	wolfSSL_CTX_free(ctx);
#else
	SSL_CTX_free(ctx);
#endif
	ctx = NULL;
}

static ssize_t mssl_read(void *cookie, char *buf, size_t len)
{
	logmsg(LOG_DEBUG, "*** [mssl] %s IN", __FUNCTION__);

	mssl_cookie_t *kuki = cookie;
	int total = 0;
	int n, err;

	do {
#ifdef USE_WOLFSSL
		n = wolfSSL_read(kuki->ssl, &(buf[total]), len - total);
#else
		n = SSL_read(kuki->ssl, &(buf[total]), len - total);
#endif
		logmsg(LOG_DEBUG, "*** [mssl] SSL_read(max=%d) returned %d", len - total, n);

#ifdef USE_WOLFSSL
		err = wolfSSL_get_error(kuki->ssl, n);
#else
		err = SSL_get_error(kuki->ssl, n);
#endif
		switch (err) {
		case SSL_ERROR_NONE:
			total += n;
			break;
		case SSL_ERROR_ZERO_RETURN:
			total += n;
			goto OUT;
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_READ:
			break;
		default:
			logmsg(LOG_DEBUG, "*** [mssl] %s: SSL error: %d", __FUNCTION__, err);
			mssl_print_err(kuki->ssl, err);
			if (total == 0)
				total = -1;
			goto OUT;
		}
#ifdef USE_WOLFSSL
	} while ((len - total > 0) && wolfSSL_pending(kuki->ssl));
#else
	} while ((len - total > 0) && SSL_pending(kuki->ssl));
#endif

OUT:
	logmsg(LOG_DEBUG, "*** [mssl] %s: returns %d", __FUNCTION__, total);
	return total;
}

static ssize_t mssl_write(void *cookie, const char *buf, size_t len)
{
	logmsg(LOG_DEBUG, "*** [mssl] %s IN", __FUNCTION__);

	mssl_cookie_t *kuki = cookie;
	unsigned int total = 0;
	int n, err;

	while (total < len) {
#ifdef USE_WOLFSSL
		n = wolfSSL_write(kuki->ssl, &(buf[total]), len - total);
#else
		n = SSL_write(kuki->ssl, &(buf[total]), len - total);
#endif

		logmsg(LOG_DEBUG, "*** [mssl] SSL_write(max=%d) returned %d", len - total, n);

#ifdef USE_WOLFSSL
		err = wolfSSL_get_error(kuki->ssl, n);
#else
		err = SSL_get_error(kuki->ssl, n);
#endif
		switch (err) {
		case SSL_ERROR_NONE:
			total += n;
			break;
		case SSL_ERROR_ZERO_RETURN:
			total += n;
			goto OUT;
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_READ:
			break;
		default:
			logmsg(LOG_DEBUG, "*** [mssl] %s: SSL error: %d", __FUNCTION__, err);
			mssl_print_err(kuki->ssl, err);
			if (total == 0)
				total = -1;

			goto OUT;
		}
	}

OUT:
	logmsg(LOG_DEBUG, "*** [mssl] %s returns %d", __FUNCTION__, total);

	return total;
}

static int mssl_seek(void *cookie, off64_t *pos, int whence)
{
	logmsg(LOG_DEBUG, "*** [mssl] %s IN", __FUNCTION__);
	errno = EIO;

	return -1;
}

static int mssl_close(void *cookie)
{
	logmsg(LOG_DEBUG, "*** [mssl] %s IN", __FUNCTION__);

	mssl_cookie_t *kuki = cookie;
	if (!kuki)
		return 0;

	if (kuki->ssl) {
#ifdef USE_WOLFSSL
		wolfSSL_shutdown(kuki->ssl);
		wolfSSL_free(kuki->ssl);
#else
		SSL_shutdown(kuki->ssl);
		SSL_free(kuki->ssl);
#endif
	}

	free(kuki);

	return 0;
}

static const cookie_io_functions_t mssl = {
	mssl_read, mssl_write, mssl_seek, mssl_close
};

static FILE *_ssl_fopen(int sd, int client, const char *name)
{
	int err, r = 0;
	mssl_cookie_t *kuki;
	FILE *f;

	logmsg(LOG_DEBUG, "*** [mssl] %s IN", __FUNCTION__);

	if ((kuki = calloc(1, sizeof(*kuki))) == NULL) {
		errno = ENOMEM;
		logmsg(LOG_DEBUG, "*** [mssl] %s: calloc failed", __FUNCTION__);
		return NULL;
	}
	kuki->sd = sd;

	/* Create new SSL object */
#ifdef USE_WOLFSSL
	if ((kuki->ssl = wolfSSL_new(ctx)) == NULL) {
#else
	if ((kuki->ssl = SSL_new(ctx)) == NULL) {
#endif
		logmsg(LOG_DEBUG, "*** [mssl] %s: SSL_new failed", __FUNCTION__);
		goto ERROR;
	}

	/* SSL structure for client authenticate after SSL_new() */
#if defined(USE_OPENSSL) || defined(USE_WOLFSSL)
 #ifdef USE_WOLFSSL
	wolfSSL_set_verify(kuki->ssl, WOLFSSL_VERIFY_NONE, NULL);
 #else
	SSL_set_verify(kuki->ssl, SSL_VERIFY_NONE, NULL);
	SSL_set_mode(kuki->ssl, SSL_MODE_AUTO_RETRY);
 #endif
#endif /* USE_OPENSSL || USE_WOLFSSL */

#ifndef USE_LIBCURL
 #if defined(USE_OPENSSL) || defined(USE_WOLFSSL)
	if (client) {
		/* Setup SNI */
  #if (OPENSSL_VERSION_NUMBER >= 0x0090806fL && !defined(OPENSSL_NO_TLSEXT)) || (defined(USE_WOLFSSL) && defined(HAVE_SNI))
		if (name && *name) {
			struct addrinfo *res, hint = { .ai_flags = AI_NUMERICHOST };
			if (getaddrinfo(name, NULL, &hint, &res) == 0)
				freeaddrinfo(res);
   #ifdef USE_WOLFSSL
			else if (wolfSSL_set_tlsext_host_name(kuki->ssl, name) != 1) {
   #else
			else if (SSL_set_tlsext_host_name(kuki->ssl, name) != 1) {
   #endif
				logmsg(LOG_DEBUG, "*** [mssl] %s: SSL_set_tlsext_host_name failed", __FUNCTION__);
				mssl_print_err(kuki->ssl, 0);
				goto ERROR;
			}
		}
  #endif
	}
 #endif
#endif /* !USE_LIBCURL */

	/* Bind the socket to SSL structure
	 * kuki->ssl : SSL structure
	 * kuki->sd  : socket_fd
	 */
#ifdef USE_WOLFSSL
	r = wolfSSL_set_fd(kuki->ssl, kuki->sd);
#else
	r = SSL_set_fd(kuki->ssl, kuki->sd);
#endif
	if (r == 0) {
#ifdef USE_WOLFSSL
		err = wolfSSL_get_error(kuki->ssl, r);
#else
		err = SSL_get_error(kuki->ssl, r);
#endif
		logmsg(LOG_DEBUG, "*** [mssl] %s: SSL_set_fd failed", __FUNCTION__);
		mssl_print_err(kuki->ssl, err);
		goto ERROR;
	}

	if (!client) {
		/* Do the SSL Handshake */
#ifdef USE_WOLFSSL
		r = wolfSSL_accept(kuki->ssl);
#else
		r = SSL_accept(kuki->ssl);
#endif
	}
#ifndef USE_LIBCURL
	else {
 #ifdef USE_WOLFSSL
		r = wolfSSL_connect(kuki->ssl);
 #else
		r = SSL_connect(kuki->ssl);
 #endif
	}
#endif /* !USE_LIBCURL */

	/* r = 0 show unknown CA, but we don't have any CA, so ignore */
	if (r < 0) {
		/* Check error in connect or accept */
#ifdef USE_WOLFSSL
		err = wolfSSL_get_error(kuki->ssl, r);
#else
		err = SSL_get_error(kuki->ssl, r);
#endif
		logmsg(LOG_DEBUG, "*** [mssl] %s: SSL_%s failed", __FUNCTION__, (client ? "connect" : "accept"));
		mssl_print_err(kuki->ssl, err);
		goto ERROR;
	}

#ifdef USE_WOLFSSL
	logmsg(LOG_DEBUG, "*** [mssl] SSL connection using %s cipher", wolfSSL_get_cipher_name(kuki->ssl));
#elif USE_OPENSSL
	logmsg(LOG_DEBUG, "*** [mssl] SSL connection using %s cipher", SSL_get_cipher(kuki->ssl));
#endif

	if ((f = fopencookie(kuki, "r+", mssl)) == NULL) {
		logmsg(LOG_DEBUG, "*** [mssl] %s: fopencookie failed", __FUNCTION__);
		goto ERROR;
	}

	logmsg(LOG_DEBUG, "*** [mssl] %s success", __FUNCTION__);
	return f;

ERROR:
	mssl_close(kuki);

	return NULL;
}

FILE *ssl_server_fopen(int sd)
{
	return _ssl_fopen(sd, 0, NULL);
}

#ifndef USE_LIBCURL
FILE *ssl_client_fopen(int sd)
{
	return _ssl_fopen(sd, 1, NULL);
}

FILE *ssl_client_fopen_name(int sd, const char *name)
{
	return _ssl_fopen(sd, 1, name);
}
#endif

#if defined(USE_OPENSSL) && !defined(SSL_OP_NO_RENEGOTIATION)
 #if OPENSSL_VERSION_NUMBER < 0x10100000L && defined(SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS)
static void ssl_info_cb(const SSL *ssl, int where, int ret)
{
	if ((where & SSL_CB_HANDSHAKE_DONE) != 0 && SSL_is_server((SSL *) ssl)) {
		/* disable renegotiation (CVE-2009-3555) */
		ssl->s3->flags |= SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS;
	}
}
 #endif
#endif /* USE_OPENSSL && !SSL_OP_NO_RENEGOTIATION */

int mssl_init_ex(char *cert, char *priv, char *ciphers)
{
	int server;

	logmsg(LOG_DEBUG, "*** [mssl] %s IN", __FUNCTION__);

	server = (cert != NULL);

#ifdef USE_WOLFSSL
	wolfSSL_Init();
	wolfSSL_add_all_algorithms();
#elif USE_OPENSSL
	SSL_library_init();
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
#endif

	/* Create the new CTX with the method 
	 * use TLSv1_server_method() / SSLv23_server_method() / wolfTLSv1_2_server_method
	 * or
	 *     TLSv1_client_method() / SSLv23_client_method() / wolfTLSv1_2_client_method
	 */
	if (server) {
#ifdef USE_WOLFSSL
		ctx = wolfSSL_CTX_new(wolfTLSv1_2_server_method());
#elif USE_OPENSSL
 #if OPENSSL_VERSION_NUMBER >= 0x10100000L
		ctx = SSL_CTX_new(TLS_server_method());
 #else
		ctx = SSL_CTX_new(SSLv23_server_method());
 #endif
#elif USE_CYASSL
		ctx = SSL_CTX_new(SSLv23_server_method());
#endif /* USE_WOLFSSL */
	}
#ifndef USE_LIBCURL
	else {
 #ifdef USE_WOLFSSL
		ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
 #elif USE_OPENSSL
  #if OPENSSL_VERSION_NUMBER >= 0x10100000L
		ctx = SSL_CTX_new(TLS_client_method());
  #else
		ctx = SSL_CTX_new(SSLv23_client_method());
  #endif
 #elif USE_CYASSL
		ctx = SSL_CTX_new(SSLv23_client_method());
 #endif /* USE_WOLFSSL */
	}
#endif /* !USE_LIBCURL */

	if (!ctx) {
		logmsg(LOG_DEBUG, "*** [mssl] SSL_CTX_new() failed");
		mssl_print_err(NULL, 0);
		return 0;
	}

#if defined(USE_OPENSSL) || defined(USE_WOLFSSL)
	/* Setup common modes */
 #ifdef USE_WOLFSSL
	wolfSSL_CTX_set_mode(ctx,
 #else
	SSL_CTX_set_mode(ctx,
 #endif
 #ifdef SSL_MODE_RELEASE_BUFFERS
				 SSL_MODE_RELEASE_BUFFERS |
 #endif
				 SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

	/* Setup common options */
 #ifdef USE_WOLFSSL
	wolfSSL_CTX_set_options(ctx, SSL_OP_ALL |
 #else
	SSL_CTX_set_options(ctx, SSL_OP_ALL |
 #endif
 #ifdef SSL_OP_NO_TICKET
				 SSL_OP_NO_TICKET |
 #endif
 #ifdef SSL_OP_NO_COMPRESSION
				 SSL_OP_NO_COMPRESSION |
 #endif
 #ifdef SSL_OP_SINGLE_DH_USE
				 SSL_OP_SINGLE_DH_USE |
 #endif
				 SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);

	/* Setup EC support */
 #ifdef NID_X9_62_prime256v1
	EC_KEY *ecdh = NULL;
  #ifdef USE_WOLFSSL
	if ((ecdh = wolfSSL_EC_KEY_new_by_curve_name(NID_X9_62_prime256v1)) != NULL) {
		wolfSSL_SSL_CTX_set_tmp_ecdh(ctx, ecdh);
		wolfSSL_EC_KEY_free(ecdh);
  #else
	if ((ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1)) != NULL) {
		SSL_CTX_set_tmp_ecdh(ctx, ecdh);
		EC_KEY_free(ecdh);
  #endif
  #ifdef SSL_OP_SINGLE_ECDH_USE
   #ifdef USE_WOLFSSL
		wolfSSL_CTX_set_options(ctx, SSL_OP_SINGLE_ECDH_USE);
   #else
		SSL_CTX_set_options(ctx, SSL_OP_SINGLE_ECDH_USE);
   #endif
  #endif
	}
 #endif
#endif /* USE_OPENSSL || USE_WOLFSSL */

	/* Setup available ciphers */
#ifndef USE_LIBCURL
	ciphers = server ? SERVER_CIPHERS : CLIENT_CIPHERS;
#else
	ciphers = SERVER_CIPHERS;
#endif

#ifdef USE_WOLFSSL
	if (ciphers && wolfSSL_CTX_set_cipher_list(ctx, ciphers) != 1) {
#else
	if (ciphers && SSL_CTX_set_cipher_list(ctx, ciphers) != 1) {
#endif
		logmsg(LOG_DEBUG, "*** [mssl] %s: SSL_CTX_set_cipher_list failed", __FUNCTION__);
		mssl_cleanup(1);
		return 0;
	}

	if (server) {
		/* Set the certificate to be used */
		logmsg(LOG_DEBUG, "*** [mssl] SSL_CTX_use_certificate_chain_file (%s)", cert);
#ifdef USE_WOLFSSL
		if (wolfSSL_CTX_use_certificate_chain_file(ctx, cert) != WOLFSSL_SUCCESS) {
#else
		if (SSL_CTX_use_certificate_chain_file(ctx, cert) <= 0) {
#endif
			logmsg(LOG_DEBUG, "*** [mssl] SSL_CTX_use_certificate_chain_file() failed");
			mssl_cleanup(1);
			return 0;
		}
		/* Indicate the key file to be used */
		logmsg(LOG_DEBUG, "*** [mssl] SSL_CTX_use_PrivateKey_file (%s)", priv);
#ifdef USE_WOLFSSL
		if (wolfSSL_CTX_use_PrivateKey_file(ctx, priv, WOLFSSL_FILETYPE_PEM) != WOLFSSL_SUCCESS) {
#else
		if (SSL_CTX_use_PrivateKey_file(ctx, priv, SSL_FILETYPE_PEM) <= 0) {
#endif
			logmsg(LOG_DEBUG, "*** [mssl] SSL_CTX_use_PrivateKey_file() failed");
			mssl_cleanup(1);
			return 0;
		}

#if defined(USE_OPENSSL) || defined(USE_WOLFSSL)
		/* Make sure the key and certificate file match */
#ifdef USE_WOLFSSL
		if (!wolfSSL_CTX_check_private_key(ctx)) {
#else
		if (!SSL_CTX_check_private_key(ctx)) {
#endif
			logmsg(LOG_DEBUG, "*** [mssl] Private key does not match the certificate public key");
			mssl_cleanup(0);
			return 0;
		}
#endif

#ifdef USE_WOLFSSL
		/* Enforce server cipher order */
		wolfSSL_CTX_set_options(ctx, WOLFSSL_OP_CIPHER_SERVER_PREFERENCE);
#elif USE_OPENSSL
 #if OPENSSL_VERSION_NUMBER >= 0x10100000L
		/* Disable TLS 1.0 & 1.1 */
		SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
 #endif
		/* Enforce server cipher order */
		SSL_CTX_set_options(ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
#endif /* USE_WOLFSSL */

		/* Disable renegotiation */
#if defined(USE_OPENSSL) || defined(USE_WOLFSSL)
 #if defined(SSL_OP_NO_RENEGOTIATION) || defined(WOLFSSL_OP_NO_RENEGOTIATION)
  #ifdef USE_WOLFSSL
		wolfSSL_CTX_set_options(ctx, WOLFSSL_OP_NO_RENEGOTIATION);
  #else
		SSL_CTX_set_options(ctx, SSL_OP_NO_RENEGOTIATION);
  #endif
 #elif OPENSSL_VERSION_NUMBER < 0x10100000L && defined(SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS)
		SSL_CTX_set_info_callback(ctx, ssl_info_cb);
 #endif
#endif /* USE_OPENSSL || USE_WOLFSSL */
	}

#ifdef USE_WOLFSSL
	wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_NONE, NULL);
#else
	SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
#endif

	logmsg(LOG_DEBUG, "*** [mssl] %s success", __FUNCTION__);

	return 1;
}

int mssl_init(char *cert, char *priv)
{
	return mssl_init_ex(cert, priv, NULL);
}

#if defined(USE_OPENSSL) && OPENSSL_VERSION_NUMBER >= 0x10100000L
static int mssl_f_exists(const char *path)
{
	struct stat st;
	return (stat(path, &st) == 0) && (!S_ISDIR(st.st_mode));
}

/*
 * compare the modulus of public key and private key
 */
int mssl_cert_key_match(const char *cert_path, const char *key_path)
{
	FILE *fp;
	X509 *x509data = NULL;
	EVP_PKEY *pkey = NULL;
	RSA *rsa_pub = NULL;
	RSA *rsa_pri = NULL;
	DSA *dsa_pub = NULL;
	DSA *dsa_pri = NULL;
	EC_KEY *ec_pub = NULL;
	EC_KEY *ec_pri = NULL;
	const EC_GROUP *ec_group = NULL;
	const EC_POINT *ec_pub_pub = NULL;
	const EC_POINT *ec_pri_pub = NULL;

	int pem = 1;
	int ret = 0;

	if (!mssl_f_exists(cert_path) || !mssl_f_exists(key_path))
		return 0;

	/* get x509 from cert file */
	fp = fopen(cert_path, "r");
	if (!fp)
		return 0;

	if (!PEM_read_X509(fp, &x509data, NULL, NULL)) {
		logmsg(LOG_DEBUG, "*** [mssl] Try to read DER format certificate");
		pem = 0;
		fseek(fp, 0, SEEK_SET);
		d2i_X509_fp(fp, &x509data);
	}
	else {
		logmsg(LOG_DEBUG, "*** [mssl] PEM format certificate");
	}

	fclose(fp);
	if (x509data == NULL) {
		logmsg(LOG_DEBUG, "*** [mssl] Load certificate failed");
		ret = 0;
		goto end;
	}

	/* get pubic key from x509 */
	pkey = X509_get_pubkey(x509data);
	if (pkey == NULL) {
		ret = 0;
		goto end;
	}
	X509_free(x509data);
	x509data = NULL;

	if (EVP_PKEY_id(pkey) == EVP_PKEY_RSA)
		rsa_pub = EVP_PKEY_get1_RSA(pkey);
	else if (EVP_PKEY_id(pkey) == EVP_PKEY_DSA)
		dsa_pub = EVP_PKEY_get1_DSA(pkey);
	else if (EVP_PKEY_id(pkey) == EVP_PKEY_EC)
		ec_pub = EVP_PKEY_get1_EC_KEY(pkey);

	EVP_PKEY_free(pkey);
	pkey = NULL;

	/* get private key from key file */
	fp = fopen(key_path, "r");
	if (!fp) {
		ret = 0;
		goto end;
	}

	if (pem)
		pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
	else
		pkey = d2i_PrivateKey_fp(fp, NULL);

	fclose(fp);

	if (pkey == NULL) {
		logmsg(LOG_DEBUG, "*** [mssl] Load private key failed");
		ret = 0;
		goto end;
	}

	if (EVP_PKEY_id(pkey) == EVP_PKEY_RSA)
		rsa_pri = EVP_PKEY_get1_RSA(pkey);
	else if (EVP_PKEY_id(pkey) == EVP_PKEY_DSA)
		dsa_pri = EVP_PKEY_get1_DSA(pkey);
	else if (EVP_PKEY_id(pkey) == EVP_PKEY_EC)
		ec_pri  = EVP_PKEY_get1_EC_KEY(pkey);

	EVP_PKEY_free(pkey);
	pkey = NULL;

	/* compare modulus */
	if (rsa_pub && rsa_pri) {
		if (BN_cmp(RSA_get0_n(rsa_pub), RSA_get0_n(rsa_pri))) {
			logmsg(LOG_DEBUG, "*** [mssl] rsa n not match");
			ret = 0;
		}
		else {
			logmsg(LOG_DEBUG, "*** [mssl] rsa n match");
			ret = 1;
		}
	}
	else if (dsa_pub && dsa_pri) {
		if (BN_cmp(DSA_get0_pub_key(dsa_pub), DSA_get0_pub_key(dsa_pri))) {
			logmsg(LOG_DEBUG, "*** [mssl] dsa modulus not match");
			ret = 0;
		}
		else {
			logmsg(LOG_DEBUG, "*** [mssl] dsa modulus match");
			ret = 1;
		}
	}
	else if (ec_pub && ec_pri) {
		ec_group = EC_KEY_get0_group(ec_pub);
		ec_pub_pub = EC_KEY_get0_public_key(ec_pub);
		ec_pri_pub = EC_KEY_get0_public_key(ec_pri);

		if (ec_group != NULL && ec_pub_pub != NULL && ec_pri_pub != NULL && EC_POINT_cmp(ec_group, ec_pub_pub, ec_pri_pub, NULL) == 0) {
			logmsg(LOG_DEBUG, "*** [mssl] ec modulus match");
			ret = 1;
		}
		else {
			logmsg(LOG_DEBUG, "*** [mssl] ec modulus not match");
			ret = 0;
		}
	}
	else {
		logmsg(LOG_DEBUG, "*** [mssl] compare failed");
	}

end:
	if (x509data)
		X509_free(x509data);
	if (pkey)
		EVP_PKEY_free(pkey);
	if (rsa_pub)
		RSA_free(rsa_pub);
	if (dsa_pub)
		DSA_free(dsa_pub);
	if (ec_pub)
		EC_KEY_free(ec_pub);
	if (rsa_pri)
		RSA_free(rsa_pri);
	if (dsa_pri)
		DSA_free(dsa_pri);
	if (ec_pri)
		EC_KEY_free(ec_pri);

	return ret;
}
#endif /* USE_OPENSSL && OPENSSL_VERSION_NUMBER >= 0x10100000L */
