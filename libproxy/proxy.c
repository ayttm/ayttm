/*
 * Ayttm
 *
 * Copyright (C) 2003, 2009 the Ayttm team
 * 
 * Ayttm is a derivative of Everybuddy
 * Copyright (C) 1998-1999, Torrey Searle
 * proxy featured by Seb C.
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

/* this is a little piece of code to handle proxy connection */
/* it is intended to : 1st handle http proxy, using the CONNECT command
 , 2nd provide an easy way to add socks support */

#include "intl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef __MINGW32__
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "proxy.h"
#include "proxy_private.h"
#include "common.h"
#include "net_constants.h"

#include <glib.h>

#ifdef __MINGW32__
#define sleep(a)		Sleep(1000*a)

#define bcopy(a,b,c)	memcpy(b,a,c)
#define bzero(a,b)		memset(a,0,b)

#define ECONNREFUSED	WSAECONNREFUSED
#endif

/* Prototypes */
static char *encode_proxy_auth_str(AyProxyData *proxy);

static ssize_t write_all(int fd, const void *buf, size_t len)
{
	const char *p = (const char *)buf;
	size_t remaining = len;
	while (remaining > 0) {
		ssize_t n = write(fd, p, remaining);
		if (n < 0) {
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
				continue;
			return -1;
		}
		if (n == 0)
			return -1;
		p += n;
		remaining -= (size_t)n;
	}
	return (ssize_t)len;
}

static ssize_t read_all(int fd, void *buf, size_t len)
{
	char *p = (char *)buf;
	size_t remaining = len;
	while (remaining > 0) {
		ssize_t n = read(fd, p, remaining);
		if (n < 0) {
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
				continue;
			return -1;
		}
		if (n == 0)
			return -1;  /* EOF before expected length */
		p += n;
		remaining -= (size_t)n;
	}
	return (ssize_t)len;
}

AyProxyData *default_proxy = NULL;

#define debug_print printf

/* 
 * External function to use to set the proxy settings
 */
int ay_proxy_set_default(AyProxyType type, const char *host, int port,
	char *username, char *password)
{
	if (!default_proxy)
		default_proxy = g_new0(AyProxyData, 1);

	default_proxy->type = type;

	if (type == PROXY_NONE) {
		if (default_proxy->host)
			free(default_proxy->host);

		if (default_proxy->username)
			free(default_proxy->username);

		if (default_proxy->password)
			free(default_proxy->password);

		g_free(default_proxy);
		default_proxy = NULL;
	} else {
		default_proxy->port = 0;

		if (host != NULL && host[0]) {
			default_proxy->host = strdup(host);
			default_proxy->port = port;
		}
		if (default_proxy->port == 0)
			default_proxy->port = 3128;

		if (username && username[0])
			default_proxy->username = strdup(username);

		if (password && password[0])
			default_proxy->password = strdup(password);

	}
#ifdef __MINGW32__
	{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 0), &wsaData);
	}
#endif
	return (0);
}

/* http://archive.socks.permeo.com/protocol/socks4.protocol */
int socks4_connect(int sock, const char *host, int port, AyProxyData *proxy)
{
	int i, packetlen;

	unsigned char *packet = NULL;
	struct addrinfo *result = NULL;

	int retval = 0;

	if (proxy->username && proxy->username[0])
		packetlen = 9 + strlen(proxy->username);
	else
		packetlen = 9;

	result = lookup_address(host, port, AF_INET);

	if (!result)
		return AY_HOSTNAME_LOOKUP_FAIL;

	packet = (unsigned char *)calloc(packetlen, sizeof(unsigned char));

	packet[0] = 4;		/* Version */
	packet[1] = 1;		/* CONNECT  */
	packet[2] = (((unsigned short)port) >> 8);	/* DESTPORT */
	packet[3] = (((unsigned short)port) & 0xff);	/* DESTPORT */

	/* DESTIP */
	bcopy(packet + 4, &(((struct sockaddr_in *)result->ai_addr)->sin_addr),
		4);

	freeaddrinfo(result);

	if (proxy->username && proxy->username[0]) {
		for (i = 0; proxy->username[i]; i++) {
			packet[i + 8] = (unsigned char)proxy->username[i];	/* AUTH      */
		}
	}
	packet[packetlen - 1] = 0;	/* END          */
	debug_print("Sending \"%s\"\n", packet);
	if (write(sock, packet, packetlen) == packetlen) {
		memset(packet, 0, packetlen);
		/* Check response - return as SOCKS4 if its valid */
		if (read(sock, packet, 9) >= 4) {
			if (packet[1] == 90) {
				return 0;
			} else if (packet[1] == 91)
				retval = AY_SOCKS4_UNKNOWN;
			else if (packet[1] == 92)
				retval = AY_SOCKS4_IDENTD_FAIL;
			else if (packet[1] == 93)
				retval = AY_SOCKS4_IDENT_USER_DIFF;
			else {
				retval = AY_SOCKS4_INCOMPATIBLE_ERROR;
			}
		} else {
			fprintf(stderr, "SOCKS4: short read\n");
		}
	}
	close(sock);

	return retval;
}

/* http://archive.socks.permeo.com/rfc/rfc1928.txt */
/* http://archive.socks.permeo.com/rfc/rfc1929.txt */

/* 
 * Removed support for datagram connections because we're not even using it now. 
 * I'll add it back if/when it is needed or if I feel like being very correct 
 * some time later...
 */
int socks5_connect(int sockfd, const char *host, int port, AyProxyData *proxy)
{
	int i;
	char buff[530];
	struct addrinfo *result = NULL;
	int j;

	buff[0] = 0x05;		/* use socks v5 */
	if (proxy->username && proxy->username[0]) {
		buff[1] = 0x02;	/* we support (no authentication & username/pass) */
		buff[2] = 0x00;	/* we support the method type "no authentication" */
		buff[3] = 0x02;	/* we support the method type "username/passw" */
	} else {
		buff[1] = 0x01;	/* we support (no authentication) */
		buff[2] = 0x00;	/* we support the method type "no authentication" */
	}

	if (write_all(sockfd, buff, 3 + ((proxy->username
				&& proxy->username[0]) ? 1 : 0)) < 0) {
		close(sockfd);
		return AY_SOCKS5_CONNECT_FAIL;
	}

	if (read_all(sockfd, buff, 2) != 2) {
		close(sockfd);
		return AY_SOCKS5_CONNECT_FAIL;
	}
	if (buff[1] == 0x00)
		;	/* no auth needed */
	else if (buff[1] == 0x02 && proxy->username && proxy->username[0])
		;	/* username/password auth */
	else {
		fprintf(stderr, "No Acceptable Methods\n");
		close(sockfd);
		return AY_SOCKS5_CONNECT_FAIL;
	}
	if (buff[1] == 0x02 && proxy->username && proxy->username[0]) {
		size_t ulen = strlen(proxy->username);
		size_t plen = proxy->password ? strlen(proxy->password) : 0;
		if (ulen > 255) ulen = 255;
		if (plen > 255) plen = 255;

		/* subneg start */
		buff[0] = 0x01;	/* subneg version  */
		buff[1] = (unsigned char)ulen;	/* username length */
		for (i = 0; i < ulen; i++) {
			buff[i + 2] = proxy->username[i];
		}
		i += 2;
		buff[i] = (unsigned char)plen;	/* password length */
		i++;
		for (j = 0; j < plen; j++) {
			buff[i + j] = proxy->password[j];
		}
		i += (j);
		buff[i] = 0;

		if (write_all(sockfd, buff, i) < 0) {
			close(sockfd);
			return AY_SOCKS5_CONNECT_FAIL;
		}

		if (read_all(sockfd, buff, 2) != 2) {
			close(sockfd);
			return AY_SOCKS5_CONNECT_FAIL;
		}

		if (buff[1] != 0)
			return AY_PROXY_PERMISSION_DENIED;
	}

	buff[0] = 0x05;		/* use socks5 */
	buff[1] = 0x01;		/* connect only SOCK_STREAM for now */
	buff[2] = 0x00;		/* reserved */
	buff[3] = 0x01;		/* ipv4 address */

	if ((result = lookup_address(host, port, AF_UNSPEC)) == NULL)
		return AY_HOSTNAME_LOOKUP_FAIL;

	memcpy(buff + 4, &(((struct sockaddr_in *)result->ai_addr)->sin_addr),
		4);
	memcpy((buff + 8), &(((struct sockaddr_in *)result->ai_addr)->sin_port),
		2);

	freeaddrinfo(result);

	if (write_all(sockfd, buff, 10) < 0) {
		close(sockfd);
		return AY_SOCKS5_CONNECT_FAIL;
	}

	if (read_all(sockfd, buff, 10) != 10) {
		close(sockfd);
		return AY_SOCKS5_CONNECT_FAIL;
	}

	if (buff[1] != 0x00) {
		fprintf(stderr, "SOCKS5 error: code %d\n", buff[1]);
		close(sockfd);
		return AY_CONNECTION_REFUSED;
	}

	return AY_NONE;
}

int http_connect(int sockfd, const char *host, int port, AyProxyData *proxy)
{
	/* step two : do  proxy tunneling init */
	char cmd[512];
	char *inputline = NULL;
	char *proxy_auth = NULL;
	int remaining = sizeof(cmd) - 1;

	remaining -= snprintf(cmd, sizeof(cmd), "CONNECT %s:%d HTTP/1.1\r\n", host, port);
	if (proxy->username && proxy->username[0]) {
		proxy_auth = encode_proxy_auth_str(proxy);

		strncat(cmd, "Proxy-Authorization: Basic ", remaining);
		remaining -= 27;
		strncat(cmd, proxy_auth, remaining);
		remaining -= strlen(proxy_auth);
		strncat(cmd, "\r\n", remaining);
		remaining -= 2;
	}
	strncat(cmd, "\r\n", remaining);
#ifdef DEBUG
	debug_print("<%s>\n", cmd);
#endif
	if (send(sockfd, cmd, strlen(cmd), 0) < 0)
		return AY_CONNECTION_REFUSED;
	if (ay_recv_line(sockfd, &inputline) < 0)
		return AY_CONNECTION_REFUSED;
#ifdef DEBUG
	debug_print("<%s>\n", inputline);
#endif
	if (!strstr(inputline, "200")) {
		/* Check if proxy authorization needed */
		if (strstr(inputline, "407")) {
			while (ay_recv_line(sockfd, &inputline) > 0) {
				free(inputline);
			}
			return AY_PROXY_AUTH_REQUIRED;
		}
		if (strstr(inputline, "403")) {
			while (ay_recv_line(sockfd, &inputline) > 0) {
				free(inputline);
			}
			return AY_PROXY_PERMISSION_DENIED;
		}
		free(inputline);
		return AY_CONNECTION_REFUSED;
	}

	while (strlen(inputline) > 1) {
		free(inputline);
		if (ay_recv_line(sockfd, &inputline) < 0) {
			return AY_CONNECTION_REFUSED;
		}
#ifdef DEBUG
		debug_print("<%s>\n", inputline);
#endif
	}
	free(inputline);

	g_free(proxy_auth);

	return 0;
}

static char *encode_proxy_auth_str(AyProxyData *proxy)
{
	char *buff = NULL;
	char *ret = NULL;

	if (proxy->username == NULL)
		return NULL;

	buff = g_strdup_printf("%s:%s", proxy->username, proxy->password);

	ret = g_base64_encode((unsigned char *)buff, strlen(buff));
	g_free (buff);

	return ret;
}
