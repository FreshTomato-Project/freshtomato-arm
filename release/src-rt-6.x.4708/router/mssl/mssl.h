/*
 *
 * Minimal CyaSSL/OpenSSL/WolfSSL Helper
 * Copyright (C) 2006-2009 Jonathan Zarate
 *
 * Licensed under GNU GPL v2 or later.
 *
 * Fixes/updates (C) 2018 - 2026 pedro
 * https://freshtomato.org/
 *
 */


#ifndef __MSSL_H__
#define __MSSL_H__

#ifdef USE_WOLFSSL
 #include <config.h>
 #include <wolfssl/options.h>
 #include <wolfssl/wolfcrypt/settings.h>
 #include <wolfssl/ssl.h>
 #include <wolfssl/error-ssl.h>
 #if 0
  wolfSSL_Debugging_ON();
 #endif
#elif USE_OPENSSL
 #include <openssl/ssl.h>
 #include <openssl/err.h>
 #include <openssl/rsa.h>
 #include <openssl/ec.h>
 #include <openssl/crypto.h>
 #include <openssl/x509.h>
 #include <openssl/pem.h>
 #include <openssl/opensslv.h>
#elif USE_CYASSL
 #include <openssl/ssl.h>
 #include <openssl/err.h>
 #include <cyassl_error.h>
#endif


extern FILE *ssl_server_fopen(int sd);
#ifndef USE_LIBCURL
 extern FILE *ssl_client_fopen(int sd);
 extern FILE *ssl_client_fopen_name(int sd, const char *name);
#endif
extern int mssl_init(char *cert, char *priv);
extern int mssl_init_ex(char *cert, char *priv, char *ciphers);
#if defined(USE_OPENSSL) && OPENSSL_VERSION_NUMBER >= 0x10100000L
 extern int mssl_cert_key_match(const char *cert_path, const char *key_path);
#endif

#endif /* __MSSL_H__ */
