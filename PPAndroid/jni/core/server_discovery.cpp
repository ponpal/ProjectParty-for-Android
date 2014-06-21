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

#define INVALID_SERVER_INFO (ServerInfo){0,0,0,0,0,0}

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
		LOGE("Could not bind socket, %d %s", errno, strerror(err));
	int broadcastEnable = 1;

	err = setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
	if(err < 0)
		LOGE("Could not enable broadcasting, %d %s", errno, strerror(err));

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
            LOGE("Could not send, %d %s", errno, strerror(err));
        struct timespec time1, time2;
        time1.tv_sec = 1;
        time1.tv_nsec = 0;
        nanosleep(&time1, &time2);
	}

    pthread_mutex_lock(&discovery->mutex);
	delete discovery->serverInfo;
	discovery->serverInfo = nullptr;
    pthread_mutex_unlock(&discovery->mutex);
	pthread_mutex_destroy(&discovery->mutex);
	delete discovery;
}

ServerDiscovery* serverDiscoveryStart()
{
    auto discovery = new ServerDiscovery();
    discovery->serverInfo = new ServerInfo[10];
    discovery->length = 0;
    discovery->capacity = 10;
    discovery->broadcastIP = platformGetBroadcastAddress();
    discovery->shouldClose = false;
    LOGI("Broadcast Address: %X", discovery->broadcastIP);
    pthread_mutex_init(&discovery->mutex, nullptr);

    pthread_t t;
    pthread_create(&t, nullptr, &serverDiscoveryTask, discovery);

    return discovery;
}

ServerInfo serverNextInfo(ServerDiscovery* discovery)
{
	ASSERT(discovery->serverInfo, "Serverdiscovery has been deleted, yet nextinfo was called.");
	pthread_mutex_lock(&discovery->mutex);
	auto info = discovery->serverInfo[discovery->length ? discovery->length - 1:0];
	if(discovery->length > 0)
	{
		discovery->length--;
	}
	else
	{
		info = INVALID_SERVER_INFO;
	}
	pthread_mutex_unlock(&discovery->mutex);
	return info;
}

void serverDiscoveryStop(ServerDiscovery* discovery)
{
	pthread_mutex_lock(&discovery->mutex);
	discovery->shouldClose = true;
	pthread_mutex_unlock(&discovery->mutex);
}


