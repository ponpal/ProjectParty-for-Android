/*
 * socket_helpers.cpp
 *
 *  Created on: 12 jul 2014
 *      Author: Lukas
 */

#include "fcntl.h"
#include "errno.h"
#include "remote_log.h"
#include "netinet/in.h"
#include "sys/socket.h"
#include "unistd.h"
#include <sys/types.h>
#include "socket_helpers.h"

bool socketConnectTimeout(int socket, uint32_t ip, uint16_t port, uint32_t msecs)
{
	socketSetBlocking(socket, false);
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(ip);
	servaddr.sin_port = htons(port);


	fd_set set;
	FD_ZERO(&set);
	FD_SET(socket, &set);

    struct timeval  timeout;
    timeout.tv_sec = msecs / 1000;
    timeout.tv_usec = (msecs % 1000) * 1000;

	auto tcperr = connect(socket, (struct sockaddr *)&servaddr, sizeof(servaddr));
	auto status = select(socket + 1, NULL, &set, NULL, &timeout);

	socketSetBlocking(socket, true);
	if(status < 0)
	{
		return false;
	}
	return true;
}

void socketSetBlocking(int socket, bool value)
{
	int flags = fcntl(socket, F_GETFL, 0);

	if(value)
		flags &= ~O_NONBLOCK;
	else
		flags |= O_NONBLOCK;

	auto err = fcntl(socket, F_SETFL, flags);
	if(err < 0)
		RLOGE("Could not set nonblocking, %d %s", errno, strerror(err));
}

void socketSendTimeout(int socket, uint32_t msecs)
{
	struct timeval  timeout;
	timeout.tv_sec = msecs / 1000;
	timeout.tv_usec = (msecs % 1000) * 1000;

	setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}

void socketRecvTimeout(int socket, uint32_t msecs)
{
	struct timeval  timeout;
	timeout.tv_sec = msecs / 1000;
	timeout.tv_usec = (msecs % 1000) * 1000;

	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}


