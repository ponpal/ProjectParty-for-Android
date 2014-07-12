/*
 * service_finder.cpp
 *
 *  Created on: 12 jul 2014
 *      Author: Lukas
 */

#include "service_finder.h"
#include "netinet/in.h"
#include "sys/socket.h"
#include "unistd.h"
#include <sys/types.h>
#include <stdio.h>
#include "errno.h"
#include "strings.h"
#include "assert.h"
#include "platform.h"
#include <fcntl.h>
#include <cstdio>
#include "socket_helpers.h"
#include "buffer.h"

#define SERVICE_MESSAGE_MAX 256
#define ANY_SERVICE_ID "ANY_NETWORK_SERVICE"

typedef struct ServiceFinder
{
	int socket;
	const char* serviceID;
	struct sockaddr_in broadcastAddr;
	serviceFound serviceFunction;
} ServiceFinder;


ServiceFinder* serviceFinderCreate(const char* serviceID, uint16_t port, serviceFound function)
{
	ServiceFinder* finder = (ServiceFinder*)malloc(sizeof(ServiceFinder));

	finder->serviceID = serviceID;
	finder->serviceFunction = function;
	finder->socket = socket(AF_INET, SOCK_DGRAM, 0);
	socketSetBlocking(finder->socket, false);

    struct sockaddr_in myaddr;
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(port);

	int err = bind(finder->socket, (struct sockaddr *)&myaddr, sizeof(myaddr));
	if(err < 0) {
		LOGE("Could not bind socket, %d %s", errno, strerror(err));
		free(finder);
		return nullptr;
	}

	bzero(&finder->broadcastAddr, sizeof(finder->broadcastAddr));
	finder->broadcastAddr.sin_family = AF_INET;
	finder->broadcastAddr.sin_addr.s_addr = htonl(platformGetBroadcastAddress());
	finder->broadcastAddr.sin_port = htons(port);

	int broadcastEnable = 1;
	err = setsockopt(finder->socket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
	if(err < 0)
	{
		RLOGE("Could not enable broadcasting, %d %s", errno, strerror(err));
		free(finder);
		return nullptr;
	}

	return finder;
}

void serviceFinderDestroy(ServiceFinder* finder)
{
	close(finder->socket);
	free(finder);
}

void serviceFinderQuery(ServiceFinder* finder)
{
	uint8_t arrayBuffer[SERVICE_MESSAGE_MAX];
	Buffer buffer = bufferWrapArray(arrayBuffer, SERVICE_MESSAGE_MAX);
	bufferWriteUTF8(&buffer, finder->serviceID);

	int err = sendto(finder->socket,
					 arrayBuffer,
				 	 buffer.length - bufferBytesRemaining(&buffer),
					 0,
				     (struct sockaddr*) &finder->broadcastAddr,
				     sizeof(finder->broadcastAddr));
	if(err < 0)
	{
        RLOGE("Could not send, %d %s", errno, strerror(err));
	}
}

bool serviceFinderPollFound(ServiceFinder* finder)
{
	uint8_t arrayBuffer[SERVICE_MESSAGE_MAX];
	Buffer buffer = bufferWrapArray(arrayBuffer, SERVICE_MESSAGE_MAX);
	auto r = recv(finder->socket, arrayBuffer, SERVICE_MESSAGE_MAX, 0);
	if(read < 0)
	{
		ASSERT(errno == EWOULDBLOCK || errno == EAGAIN, "Error in receiving logic");
		return false;
	}
	else
	{
		buffer.length = r;
		//Read the name!
		char name[SERVICE_MESSAGE_MAX];
		auto size = bufferReadShort(&buffer);
		bufferReadBytes(&buffer, (uint8_t*)name, size);
		name[size] = '\0';

		if(strcmp(name, ANY_SERVICE_ID) != 0 &&
		   strcmp(name, finder->serviceID) != 0)
		{
			//Failed to find the service.
			return false;
		}

		finder->serviceFunction(name, &buffer);
	}

	return true;
}
