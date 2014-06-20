/*
 * server_discovery.h
 *
 *  Created on: Jun 20, 2014
 *      Author: Gustav
 */

#ifndef SERVER_DISCOVERY_H_
#define SERVER_DISCOVERY_H_

#define SERVER_DISCOVERY_PORT 12345

#include "pthread.h"

typedef struct {
	uint32_t ip;
	uint16_t port;
} ServerInfo;

typedef struct {
	pthread_mutex_t mutex;
	ServerInfo* serverInfo;
	size_t capacity, length;
	int socket;
	bool shouldClose;
	uint32_t broadcastIP;
} ServerDiscovery;

ServerDiscovery* serverDiscoveryStart();
ServerInfo* serverQueryInfos(ServerDiscovery* discovery, int* length);
void serverDiscoveryStop(ServerDiscovery* discovery);


#endif /* SERVER_DISCOVERY_H_ */
