/*
 * network_internal.h
 *
 *  Created on: Feb 21, 2014
 *      Author: Lukas_2
 */

#ifndef NETWORK_INTERNAL_H_
#define NETWORK_INTERNAL_H_

#include <jni.h>
#include <android_native_app_glue.h>
#include "stdint.h"
#include "buffer.h"

#define SERVER_REFUSED_RECONNECT 250
#define COULD_NOT_RECONNECT 500

extern "C"
{
	enum
	{
		NETWORK_ALIAS      = 0,
		NETWORK_SENSOR     = 1,
		NETWORK_FILE       = 2,
		NETWORK_ALLFILES   = 3,
		NETWORK_FILERELOAD = 4,
		NETWORK_LUALOG	   = 5, //unused
		NETWORK_TRANSITION = 6,
		NETWORK_HEARTBEAT  = 7,
		NETWORK_SHUTDOWN   = 8
	};

	typedef struct
	{
		Buffer* in_;
		Buffer* out;
		Buffer* uout;
	} Network;

	int networkSend(Network* network);
	int networkUnreliableSend(Network* network);
	int networkReceive(Network* network, uint8_t* buffer, uint32_t size);

	int networkIsAlive(Network* network);
	int networkConnect(Network* network);
	int networkReconnect(Network* network);
	int networkDisconnect(Network* network);

	int networkShutdown(Network* network);
}

void networkServiceClass(jclass clazz);
Network* networkInitialize(android_app* app);
void networkDelete(Network* network);

#endif /* NETWORK_INTERNAL_H_ */
