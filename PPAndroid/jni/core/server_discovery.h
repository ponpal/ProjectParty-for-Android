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

extern "C" {

typedef struct {
	char serverName[58];
	char gameName[58];
	uint32_t serverIP;
	uint16_t contentPort;
	uint16_t serverTCPPort;
	uint16_t serverUDPPort;
} ServerInfo;

typedef struct {
	pthread_mutex_t mutex;
	ServerInfo* serverInfo;
	size_t capacity, start, end;
	int socket;
	bool shouldClose;
	uint32_t broadcastIP;
} ServerDiscovery;

ServerDiscovery* serverDiscoveryStart();
ServerInfo serverNextInfo(ServerDiscovery* discovery);
void serverDiscoveryStop(ServerDiscovery* discovery);
}

#endif /* SERVER_DISCOVERY_H_ */
