/*
 * network.cpp
 *
 *  Created on: Feb 21, 2014
 *      Author: Lukas_2
 */

#include "assert.h"
#include "network.h"
#include <android_native_app_glue.h>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>

#include "JNIHelper.h"
#include <netinet/tcp.h>
#include <fcntl.h>
#include "socket_stream.h"

#define __STDC_FORMAT_MACROS

#include <inttypes.h>

Network* networkCreate(size_t bufferSize, void (*handleMessage)(Buffer* buffer, uint16_t messageID))
{
	auto network = new Network();
	network->in_ = bufferCreate(bufferSize);
	network->uin = bufferCreate(bufferSize);
	network->out = bufferCreate(bufferSize);
	network->uout = bufferCreate(bufferSize);
	network->out->length = network->out->capacity;
	network->uout->length = network->uout->capacity;
	network->udpSocket = 0;
	network->tcpSocket = 0;
	network->remoteIP = 0;
	network->remoteUdpPort = 0;
	network->sessionID = 0;
	network->handleMessage = handleMessage;

	return network;
}

void networkDestroy(Network* network)
{
	bufferDestroy(network->in_);
	bufferDestroy(network->out);
	bufferDestroy(network->uout);
	delete network;
}

static void setNonBlocking(int socket)
{
	int flags = fcntl(socket, F_GETFL, 0);
	flags |= O_NONBLOCK;
	auto err = fcntl(socket, F_SETFL, flags);
	if(err < 0)
		LOGE("Could not set nonblocking, %d %s", errno, strerror(err));

}

int networkConnect(Network* network, uint32_t ip, uint16_t udpPort, uint16_t tcpPort)
{
	int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in myaddr;
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

	int err = bind(udpSocket, (struct sockaddr *)&myaddr, sizeof(myaddr));
	if(err < 0)
		LOGE("Could not bind socket, %d %s", errno, strerror(err));

	int tcpSocket;
	struct sockaddr_in servaddr;
	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(ip);
	servaddr.sin_port = htons(tcpPort);
	auto tcperr = connect(tcpSocket, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (tcperr)
	{
		LOGE("Could not connect, error is: %d %s", errno, strerror(errno));
		return tcperr;
	}

	int i = 1;
	setsockopt(tcpSocket, IPPROTO_TCP, TCP_NODELAY, (void *)&i, sizeof(i));
	SocketStream* stream = streamCreate(tcpSocket, 8);
	auto sessionID = streamReadLong(stream);
	send(tcpSocket, &sessionID, sizeof(sessionID), 0);

	network->sessionID = sessionID;
	LOGI("SessionID: %" PRIu64, sessionID);
	network->tcpSocket = tcpSocket;
	network->remoteIP = ip;
	network->udpSocket = udpSocket;
	network->remoteUdpPort = udpPort;

	streamDestroy(stream);

	setNonBlocking(tcpSocket);
	setNonBlocking(udpSocket);

	return 0;
}

int networkReconnect(Network* network)
{
	ASSERT(false, "not yet implemented");
	return 0;
}

void networkDisconnect(Network* network)
{
	LOGI("Disconnecting...");
	if(network->tcpSocket == 0)
		return;
	close(network->tcpSocket);
	close(network->udpSocket);
	network->tcpSocket = 0;
	network->udpSocket = 0;
	LOGI("Disconnected.");
}

int bufferReceive(int socket, Buffer* buffer)
{
	size_t length = bufferBytesRemaining(buffer);
	memmove(buffer->base, buffer->ptr, length);
	buffer->ptr = buffer->base;
	auto r = recv(socket, buffer->ptr + length, buffer->capacity - length, 0);
	if(r<0) {
		ASSERT(errno == EWOULDBLOCK || errno == EAGAIN, "Error in receiving logic");
		buffer->length = length;
	} else {
		buffer->length = r + length;
	}
	return r;
}

static int networkTCPReceive(Network* network)
{
	return bufferReceive(network->tcpSocket, network->in_);
}

static int networkUDPReceive(Network* network)
{
	return bufferReceive(network->udpSocket, network->uin);
}

int networkTCPSend(Network* network)
{
	auto toSend = (uint32_t)(network->out->ptr - network->out->base);
	auto r = send(network->tcpSocket, network->out->base, toSend, 0);
	network->out->ptr = network->out->base;
	network->out->length = network->out->capacity;
	return r;
}

int networkUDPSend(Network* network)
{

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(network->remoteIP);
	servaddr.sin_port = htons(network->remoteUdpPort);

	auto r = sendto(network->udpSocket, network->uout->base, network->uout->length, 0,
			(struct sockaddr*) &servaddr, sizeof(servaddr));
	network->uout->ptr = network->uout->base;
	network->uout->length = 0;
	return r;
}

int networkSend(Network* network)
{
	auto result = networkUDPSend(network);
	if(result == -1)
	{
		LOGE("Could not send UDP, error is: %d %s", errno, strerror(errno));
		return result;
	}
	result = networkTCPSend(network);
	if(result == -1)
	{
		LOGE("Could not send TCP, error is: %d %s", errno, strerror(errno));
		return result;
	}
	return 0;
}

bool networkIsAlive(Network* network)
{
	int error_code;
	socklen_t len = sizeof(error_code);
	auto r = getsockopt(network->tcpSocket, SOL_SOCKET, SO_ERROR, &error_code, &len);
	r |= getsockopt(network->udpSocket, SOL_SOCKET, SO_ERROR, &error_code, &len);
	if(r) {
		LOGE("Network is not alive! Error: %d %s", errno, strerror(errno));
	}
	return r == 0;
}

void networkSendLogMessage(Network* network, const char* message)
{
	bufferWriteShort(network->out, sizeof(uint16_t) + strlen(message));
	bufferWriteShort(network->out, NETWORK_OUT_LOG);
	bufferWriteUTF8(network->out, message);
	networkSend(network);
}

static void handleSystemMessage(Buffer* buffer)
{
	auto id = bufferReadByte(buffer);
    LOGI("system messageID: %d", id);
    if (id == NETWORK_IN_SHUTDOWN) {
        LOGE("Shutting down");
        gameFinish();
    }
}

static bool readMessage(Network* network, Buffer* buffer) {
	auto remaining = bufferBytesRemaining(buffer);
	LOGI("Remaining: %d", remaining);
	if (remaining < 4)
		return false;
	auto messageSize = bufferReadShort(buffer);
	LOGI("MessageSize: %d", (uint32_t)messageSize);

	if (remaining - 2 < messageSize) {
		buffer->ptr -= 2;
		return false;
    }
	auto messageID = bufferReadShort(buffer);
	LOGI("messageID: %d", (uint32_t)messageID);

	if (messageID == 0) {
        handleSystemMessage(buffer);
	} else {
		network->handleMessage(buffer, messageID);
	}

	uint32_t expected = remaining - (messageSize + 2);
	uint32_t actual = bufferBytesRemaining(buffer);

	ASSERTF(expected == actual,"Faulty message. Expected: %d, Actual: %d", expected, actual);
	return true;
}

int networkReceive(Network* network) {
    auto count = networkTCPReceive(network);
    if (count == -1) return -1;
    while (readMessage(network, network->in_)) { }

    count = networkUDPReceive(network);
    if (count == -1) return -1;
    while (readMessage(network, network->uin)) { }
    return 0;
}
