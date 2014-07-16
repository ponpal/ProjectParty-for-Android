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
	char* serviceID;
	struct sockaddr_in broadcastAddr;
	serviceFound serviceFunction;
} ServiceFinder;


ServiceFinder* serviceFinderCreate(const char* serviceID, uint16_t port, serviceFound function)
{
	ServiceFinder* finder = (ServiceFinder*)malloc(sizeof(ServiceFinder));

	char* serviceIDCpy = (char*)malloc(strlen(serviceID) + 1);
	memcpy(serviceIDCpy, serviceID, strlen(serviceID) + 1);

	finder->serviceID = serviceIDCpy;
	finder->serviceFunction = function;
	finder->socket = socket(AF_INET, SOCK_DGRAM, 0);
	socketSetBlocking(finder->socket, false);

    struct sockaddr_in myaddr;
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = 0;

	int err = bind(finder->socket, (struct sockaddr *)&myaddr, sizeof(myaddr));
	if(err < 0) {
		LOGE("Could not bind socket, %d %s", errno, strerror(err));
		free(finder);
		free(serviceIDCpy);
		return 0;
	}

	bzero(&finder->broadcastAddr, sizeof(finder->broadcastAddr));
	finder->broadcastAddr.sin_family = AF_INET;
	finder->broadcastAddr.sin_addr.s_addr = htonl(platformGetBroadcastAddress());
	finder->broadcastAddr.sin_port = htons(port);

	int broadcastEnable = 1;
	err = setsockopt(finder->socket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
	if(err < 0)
	{
		LOGE("Could not enable broadcasting, %d %s", errno, strerror(err));
		free(finder);
		free(serviceIDCpy);
		return 0;
	}

	return finder;
}

void serviceFinderDestroy(ServiceFinder* finder)
{
	close(finder->socket);
	free(finder->serviceID);
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
        LOGE("Could not send, %d %s", errno, strerror(err));
	}
}

bool serviceFinderPollFound(ServiceFinder* finder)
{
	uint8_t arrayBuffer[SERVICE_MESSAGE_MAX];
	Buffer buffer = bufferWrapArray(arrayBuffer, SERVICE_MESSAGE_MAX);
	auto r = recv(finder->socket, arrayBuffer, SERVICE_MESSAGE_MAX, 0);
	if(r < 0)
	{
		ASSERT(errno == EWOULDBLOCK || errno == EAGAIN, "Error in receiving logic");
		return false;
	}
	else
	{

		buffer.length = r;
		auto name = bufferReadTempUTF8(&buffer);


		if(strcmp(name, finder->serviceID) != 0 &&
		   strcmp(finder->serviceID, ANY_SERVICE_ID) != 0)
		{
			//Failed to find the service.
			return false;
		}

		finder->serviceFunction(name, &buffer);
	}

	return true;
}
