/*
 * socket_helpers.h
 *
 *  Created on: 12 jul 2014
 *      Author: Lukas
 */

#ifndef SOCKET_HELPERS_H_
#define SOCKET_HELPERS_H_

#include "buffer.h"

extern "C"
{
	//Socket common
	int socketCreate(int type);
	bool socketBind(int socket, uint32_t ip, uint16_t port);
	void socketSetBlocking(int socket, bool value);
	bool socketIsBlocking(int socket);
	void socketSendTimeout(int socket, uint32_t msecs);
	void socketRecvTimeout(int socket, uint32_t mescs);
	void socketDestroy(int socket);

	//For tcp sockets
	bool socketConnect(int socket, uint32_t ip, uint16_t port, uint32_t msecs);

	//Need non-blocking connect.
	typedef void (*connectedCallback)(int socket,  bool result);
	void socketAsyncConnect(int socket, uint32_t ip,
							uint16_t port, uint32_t msecs,
							connectedCallback callback);

	bool socketTCPSend(int socket, Buffer* toSend);
	void socketTcpNoDelay(int socket, bool value);

	//For udp sockets
	bool socketSend(int socket, Buffer* toSend, uint32_t ip, uint16_t port);
	bool socketReceive(int socket, Buffer* buffer);
}

#endif /* SOCKET_HELPERS_H_ */
