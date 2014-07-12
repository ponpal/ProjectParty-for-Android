/*
 * server_discovery.cpp
 *
 *  Created on: Jun 20, 2014
 *      Author: Gustav
 */

#include "netinet/in.h"
#include "sys/socket.h"
#include "unistd.h"
#include <sys/types.h>
#include <stdio.h>
#include "JNIHelper.h"
#include "errno.h"
#include "pthread.h"
#include "server_discovery.h"
#include "strings.h"
#include "assert.h"
#include "platform.h"
#include <sys/time.h>
#include "socket_stream.h"

#define INVALID_SERVER_INFO (ServerInfo){0,0,0,0,0,0}

static void readServerInfo(ServerDiscovery* discovery, Buffer* buffer, uint32_t ip)
{
    auto hostNameLength = bufferReadShort(buffer);
    auto serverInfo = &discovery->serverInfo[discovery->end];
    bufferReadBytes(buffer, (uint8_t*)serverInfo->serverName, (uint32_t)hostNameLength);
    serverInfo->serverName[hostNameLength] = '\0';
    auto gameNameLength = bufferReadShort(buffer);
    bufferReadBytes(buffer, (uint8_t*)serverInfo->gameName, (uint32_t)gameNameLength);
    serverInfo->gameName[gameNameLength] = '\0';
    serverInfo->contentPort = bufferReadShort(buffer);
    serverInfo->serverTCPPort = bufferReadShort(buffer);
    serverInfo->serverUDPPort = bufferReadShort(buffer);
    serverInfo->serverIP = ip;

    discovery->end = (discovery->end + 1) % discovery->capacity;
    if(discovery->end == discovery->start)
            discovery->start = (discovery->start + 1) % discovery->capacity;
    //RLOGW("Server name: %s", serverInfo->serverName);
    //RLOGW("Game name: %s", serverInfo->gameName);
    //RLOGW("TCP port: %d", (uint32_t)serverInfo->serverTCPPort);
    //RLOGW("UDP port: %d", (uint32_t)serverInfo->serverUDPPort);
    //RLOGW("Content port: %d", (uint32_t)serverInfo->contentPort);
    //RLOGW("Server IP: %x", serverInfo->serverIP);
}

void* serverDiscoveryTask(void* args)
{
	auto discovery = (ServerDiscovery*) args;

    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in myaddr;
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

	struct sockaddr_in broadcastaddr;
	bzero(&broadcastaddr, sizeof(broadcastaddr));
	broadcastaddr.sin_family = AF_INET;
	broadcastaddr.sin_addr.s_addr = htonl(discovery->broadcastIP);
	broadcastaddr.sin_port = htons(SERVER_DISCOVERY_PORT);

	int err = bind(udpSocket, (struct sockaddr *)&myaddr, sizeof(myaddr));
	if(err < 0)
		RLOGE("Could not bind socket, %d %s", errno, strerror(err));
	int broadcastEnable = 1;

	err = setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
	if(err < 0)
		RLOGE("Could not enable broadcasting, %d %s", errno, strerror(err));

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	err = setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	if(err < 0)
		RLOGE("Could not enable broadcasting, %d %s", errno, strerror(err));
	struct sockaddr_in recvAddr;
	auto buffer = bufferCreate(1024);
	while(true)
	{
        pthread_mutex_lock(&discovery->mutex);
        auto shouldClose = discovery->shouldClose;
        pthread_mutex_unlock(&discovery->mutex);
        if (shouldClose)
        	break;
		uint32_t broadcastAddress = 21400;
		err = sendto(udpSocket, (void*)&broadcastAddress, sizeof(broadcastAddress), 0,
				 (struct sockaddr*) &broadcastaddr, sizeof(broadcastaddr));
        if(err < 0)
            RLOGE("Could not send, %d %s", errno, strerror(err));
        while(true) {

            bzero(&recvAddr, sizeof(recvAddr));
            socklen_t len = sizeof(recvAddr);

            err = recvfrom(udpSocket, buffer->ptr, buffer->capacity, 0,
                    (struct sockaddr*) &recvAddr, &len);

            if(err == -1) {
                ASSERTF(errno == EAGAIN || errno == EWOULDBLOCK,
                    "Error in server discovery. errno: %d, error: %s", errno, strerror(err));
                break;
            }

            buffer->length = err;

            pthread_mutex_lock(&discovery->mutex);
            readServerInfo(discovery, buffer, htonl(recvAddr.sin_addr.s_addr));
            pthread_mutex_unlock(&discovery->mutex);
            buffer->ptr = buffer->base;
        }
	}

    pthread_mutex_lock(&discovery->mutex);
	delete [] discovery->serverInfo;
	discovery->serverInfo = nullptr;
    pthread_mutex_unlock(&discovery->mutex);
	pthread_mutex_destroy(&discovery->mutex);
	delete discovery;

	return 0;
}

ServerDiscovery* serverDiscoveryStart()
{
    auto discovery = new ServerDiscovery();
    discovery->serverInfo = new ServerInfo[10];
    discovery->start = discovery->end = 0;
    discovery->capacity = 10;
    discovery->broadcastIP = platformGetBroadcastAddress();
    discovery->shouldClose = false;
    RLOGW("Broadcast Address: %X", discovery->broadcastIP);
    pthread_mutex_init(&discovery->mutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, &serverDiscoveryTask, discovery);

    return discovery;
}

ServerInfo serverNextInfo(ServerDiscovery* discovery)
{
	ASSERT(discovery->serverInfo, "Serverdiscovery has been deleted, yet nextinfo was called.");
	pthread_mutex_lock(&discovery->mutex);
	auto info = discovery->serverInfo[discovery->start];
	if(discovery->start == discovery->end)
		info = INVALID_SERVER_INFO;
	else
        discovery->start = (discovery->start + 1) % discovery->capacity;
	pthread_mutex_unlock(&discovery->mutex);
	return info;
}

void serverDiscoveryStop(ServerDiscovery* discovery)
{
	pthread_mutex_lock(&discovery->mutex);
	discovery->shouldClose = true;
	pthread_mutex_unlock(&discovery->mutex);
}


