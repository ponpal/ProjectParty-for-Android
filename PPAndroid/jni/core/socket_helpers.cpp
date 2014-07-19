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

int socketCreate(int type)
{
	return socket(AF_INET, type, 0);
}

void socketDestroy(int sock)
{
	close(sock);
}

bool socketBind(int sock, uint32_t ip, uint16_t port)
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);

	int err = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if(err < 0) {
		RLOGE("Could not bind socket, %d %s", errno, strerror(err));
		return false;
	}

	return true;
}

bool socketConnect(int socket, uint32_t ip, uint16_t port, uint32_t msecs)
{
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(ip);
	servaddr.sin_port = htons(port);

	if(msecs != 0)
	{
		socketSetBlocking(socket, false);
		fd_set set;
		FD_ZERO(&set);
		FD_SET(socket, &set);

		struct timeval  timeout;
		timeout.tv_sec = msecs / 1000;
		timeout.tv_usec = (msecs % 1000) * 1000;

		auto err = connect(socket, (struct sockaddr *)&servaddr, sizeof(servaddr));
		auto status = select(socket + 1, NULL, &set, NULL, &timeout);

		socketSetBlocking(socket, true);
		if(status < 0)
		{
			RLOGE("Could not connect socket, %d %s", errno, strerror(errno));
			return false;
		}
	}
	else
	{
		auto err = connect(socket, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if(err < 0)
		{
			RLOGE("Could not bind socket, %d %s", errno, strerror(errno));
			return false;
		}
	}

	return true;
}

bool socketReceive(int sock, Buffer* buffer)
{

	size_t length = bufferBytesRemaining(buffer);
	memmove(buffer->base, buffer->ptr, length);
	buffer->ptr = buffer->base;
	auto r = recv(sock, buffer->ptr + length, buffer->capacity - length, 0);
	if(r<0) {
		if(errno != EWOULDBLOCK && errno != EAGAIN)
		{
			RLOGE("Error in socket receive. Error: %d %s", errno, strerror(errno));
			return false;
		}
		buffer->length = length;
	} else {
		buffer->length = r + length;
	}

	return true;
}

bool socketSend(int sock, Buffer* buffer, uint32_t ip, uint16_t port)
{
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);

	auto toSend = (uint32_t)(buffer->ptr - buffer->base);
	auto r = sendto(sock, buffer->base, toSend, 0, (struct sockaddr*)&addr, sizeof(addr));
	if(r < 0)
	{
		if(errno != EWOULDBLOCK && errno != EAGAIN)
		{
			RLOGE("Error in socket receive. Error: %d %s", errno, strerror(errno));
			return false;
		}
	}
	else
	{
		//Did not send entire buffer
		if(toSend != r)
		{
			//Shift down the remaining data in the buffer
			memmove(buffer->base, buffer->base + r, toSend - r);
		}

		buffer->ptr = buffer->base + toSend - r;
	}
	buffer->length = buffer->capacity;
	return true;
}

bool socketTCPSend(int sock, Buffer* buffer)
{
	auto toSend = (uint32_t)(buffer->ptr - buffer->base);
	auto r = send(sock, buffer->base, toSend, 0);
	if(r < 0)
	{
		if(errno != EWOULDBLOCK && errno != EAGAIN)
		{
			RLOGE("Error in socket receive. Error: %d %s", errno, strerror(errno));
			return false;
		}
	}
	else
	{
		//Did not send entire buffer
		if(toSend != r)
		{
			//Shift down the remaining data in the buffer
			memmove(buffer->base, buffer->base + r, toSend - r);
		}

		buffer->ptr = buffer->base + toSend - r;
	}
	buffer->length = buffer->capacity;
	return true;
}

bool socketIsBlocking(int socket)
{
	int flags = fcntl(socket, F_GETFL, 0);
	return (flags & O_NONBLOCK) != O_NONBLOCK;
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
