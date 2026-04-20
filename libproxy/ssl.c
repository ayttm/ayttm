/*
 * Ayttm
 *
 * Copyright (C) 2003,2009, the Ayttm team
 *
 * this code is derivative of Sylpheed-claws
 * Copyright (C) 1999-2003, Hiroyuki Yamamoto
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "config.h"

#ifdef HAVE_OPENSSL
#include <glib.h>
#include "ssl.h"
#include "ssl_certificate.h"

/* OpenSSL >= 1.1.1 is required (enforced in configure.ac and meson.build).
 * TLS 1.3 support (TLS1_3_VERSION) requires OpenSSL 1.1.1+.
 * Threading is handled internally by OpenSSL 1.1.0+; no locking callbacks
 * needed. SSL_library_init() and SSL_load_error_strings() are no-ops. */

#include "debug.h"
#include <string.h>
#include "globals.h"
#include "networking.h"

static gboolean ssl_inited = FALSE;

/* Global system initialization */
void ssl_init(void)
{
	/* static GMutex is zero-initialized; GLib 2.32+ guarantees this is valid.
	 * GStaticMutex and g_mutex_new() were removed in GLib 2.32. */
	static GMutex ssl_init_lock;

	g_mutex_lock(&ssl_init_lock);

	if (ssl_inited)
		goto done;

	ssl_inited = TRUE;

done:
	g_mutex_unlock(&ssl_init_lock);
}

SSL *ssl_get_socket(int sock, const char *host, int port, void *data)
{
	SSL_CTX *ssl_ctx = NULL;

	if (!ssl_inited)
		ssl_init();

	/* Use TLS_client_method() which supports TLS 1.2+ only.
	 * SSLv23_client_method() is deprecated in OpenSSL 1.1.0 and
	 * removed in OpenSSL 3.x; it also allowed negotiation of
	 * obsolete protocol versions (SSLv2, SSLv3). */
	ssl_ctx = SSL_CTX_new(TLS_client_method());
	if (ssl_ctx == NULL) {
		eb_debug(DBG_CORE, "Error creating SSL context\n");
		return NULL;
	}

	/* Enforce a minimum of TLS 1.2 — TLS 1.0 and 1.1 are deprecated
	 * (RFC 8996) and should not be negotiated. */
	SSL_CTX_set_min_proto_version(ssl_ctx, TLS1_2_VERSION);

	/* Load default CA certificate paths; if this fails, TLS connections
	 * may fail later with unhelpful errors or silently skip verification. */
	if (!SSL_CTX_set_default_verify_paths(ssl_ctx)) {
		eb_debug(DBG_CORE,
			"Warning: SSL_CTX_set_default_verify_paths() failed; "
			"CA verification may not work correctly\n");
	}

	/* Require peer certificate verification to prevent MITM attacks.
	 * Connections to servers with invalid or untrusted certificates
	 * will be rejected at SSL_connect() time. */
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);

	{
		SSL *ret = ssl_init_socket_with_method(sock, host, port, ssl_ctx,
			SSL_METHOD_TLS, data);
		/* SSL* holds its own reference to ssl_ctx; release ours now. */
		SSL_CTX_free(ssl_ctx);
		return ret;
	}
}

SSL *ssl_init_socket_with_method(int sock, const char *host, int port,
	SSL_CTX *ssl_ctx, SSLMethod method, void *data)
{
	X509 *server_cert;
	SSL *ssl;
	int res = 0;

	ssl = SSL_new(ssl_ctx);

	if (ssl == NULL) {
		eb_debug(DBG_CORE, "Error creating ssl context\n");
		return NULL;
	}

	switch (method) {
	case SSL_METHOD_TLSv1_2:
		eb_debug(DBG_CORE, "Setting TLS 1.2 client method\n");
		SSL_set_ssl_method(ssl, TLS_client_method());
		SSL_set_min_proto_version(ssl, TLS1_2_VERSION);
		SSL_set_max_proto_version(ssl, TLS1_2_VERSION);
		break;
	case SSL_METHOD_TLSv1_3:
		eb_debug(DBG_CORE, "Setting TLS 1.3 client method\n");
		SSL_set_ssl_method(ssl, TLS_client_method());
		SSL_set_min_proto_version(ssl, TLS1_3_VERSION);
		SSL_set_max_proto_version(ssl, TLS1_3_VERSION);
		break;
	case SSL_METHOD_TLS:
	default:
		eb_debug(DBG_CORE, "Setting TLS client method (>= 1.2)\n");
		SSL_set_ssl_method(ssl, TLS_client_method());
		SSL_set_min_proto_version(ssl, TLS1_2_VERSION);
		break;
	}

	SSL_set_fd(ssl, sock);
	if ((res = SSL_connect(ssl)) == -1) {
		eb_debug(DBG_CORE, ("SSL connect failed (%s)\n"),
			ERR_error_string(SSL_get_error(ssl, res), NULL));
		res = SSL_get_error(ssl, res);
		switch (res) {
		case SSL_ERROR_NONE:
			printf("SSL_ERROR_NONE\n");
			break;
		case SSL_ERROR_WANT_READ:
			printf("SSL_ERROR_WANT_READ\n");
			break;
		case SSL_ERROR_WANT_WRITE:
			printf("SSL_ERROR_WANT_WRITE\n");
			break;
		case SSL_ERROR_ZERO_RETURN:
			printf("SSL_ERROR_ZERO_RETURN\n");
			break;
		default:
			printf("DEFAULT\n");
			break;
		}
		SSL_free(ssl);
		return NULL;
	}

	/* Get the cipher */

	eb_debug(DBG_CORE, "SSL connection using %s\n", SSL_get_cipher(ssl));

	/* Get server's certificate (note: beware of dynamic allocation) */
	if ((server_cert = SSL_get_peer_certificate(ssl)) == NULL) {
		eb_debug(DBG_CORE,
			"server_cert is NULL ! this _should_not_ happen !\n");
		SSL_free(ssl);
		return NULL;
	}

	if (!ssl_certificate_check(server_cert, host, port, data)) {
		X509_free(server_cert);
		SSL_free(ssl);
		return NULL;
	}

	X509_free(server_cert);

	return ssl;
}

void ssl_done_socket(SSL *ssl)
{
	if (ssl)
		SSL_free(ssl);
}

int ssl_read(SSL *ssl, char *buf, int len)
{
	return SSL_read(ssl, buf, len);
}

int ssl_write(SSL *ssl, const char *buf, int len)
{
	return SSL_write(ssl, buf, len);
}
#endif
