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

extern "C"
{
	enum
	{
		NETWORK_ALIAS   = 0,
		NETWORK_SENSOR  = 1,
		NETWORK_FILE    = 2
	};

	typedef struct
	{
		Buffer* in_;
		Buffer* out;
	} Network;

	int networkSend(Network* network);
	int networkReceive(Network* network);

	int networkIsAlive(Network* network);
	int networkConnect(Network* network);
	int networkReconnect(Network* network);

	int networkShutdown(Network* network);
}

void networkServiceClass(jclass clazz);
Network* networkInitialize(android_app* app);

#endif /* NETWORK_INTERNAL_H_ */
