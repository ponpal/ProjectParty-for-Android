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
#include "errno.h"
#include "sys/socket.h"
#include <netinet/in.h>
#include "unistd.h"
#include "stdio.h"
#include "buffer.h"
#include "sys/types.h"

#define SERVER_REFUSED_RECONNECT 250
#define COULD_NOT_RECONNECT 500

enum {
	NETWORK_OUT_LOG = 0
};

enum {
	NETWORK_IN_SETUP_FILE_TRANSFER = 0,
	NETWORK_IN_SHUTDOWN = 1
};

extern "C"
{

	typedef struct
	{
		Buffer* in_;
		Buffer* uin;
		Buffer* out;
		Buffer* uout;
		uint32_t remoteIP;
		uint16_t remoteUdpPort;
		int udpSocket;
		int tcpSocket;
		uint64_t sessionID;
		void (*handleMessage)(Buffer* buffer, uint16_t messageID);
	} Network;

    Network* networkCreate(size_t bufferSize);
    void networkDestroy(Network* network);

	int networkSend(Network* network);
	int networkUnreliableSend(Network* network);
	int networkReceive(Network* network);
	int networkUnreliableReceive(Network* network);

	bool networkIsAlive(Network* network);
	int networkConnect(Network* network, uint32_t ip, uint16_t udpPort, uint16_t tcpPort);
	int networkReconnect(Network* network);
	void networkDisconnect(Network* network);

	void networkSendLogMessage(Network* network, const char* message);
}
#endif /* NETWORK_INTERNAL_H_ */
