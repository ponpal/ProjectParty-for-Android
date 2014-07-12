/*
 * remote_log.cpp
 *
 *  Created on: 10 jul 2014
 *      Author: Lukas
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
#include "remote_log.h"
#include <fcntl.h>
#include <cstdio>

static std::string gLoggingID;
static int listener = 0, tcpSocket = 0;
static uint16_t gPort = 0;
static bool isConnected = false;
static bool isInitialized = false;
static char text_buffer[1024];

struct BroadcastInfo
{
	uint32_t ip;
	uint16_t port;
};

static void setNonBlocking(int socket)
{
	int flags = fcntl(socket, F_GETFL, 0);
	flags |= O_NONBLOCK;
	auto err = fcntl(socket, F_SETFL, flags);
	if(err < 0)
		LOGE("Could not set nonblocking, %d %s", errno, strerror(err));

}

static void setBlocking(int socket)
{
	int flags = fcntl(socket, F_GETFL, 0);
	flags &= ~O_NONBLOCK;
	auto err = fcntl(socket, F_SETFL, flags);
	if(err < 0)
		LOGE("Could not set blocking, %d %s", errno, strerror(err));
}

static void shutdownConnection()
{
	LOGI("TCP SOCKET IS %d", tcpSocket);
	if(tcpSocket != 0) {
		LOGI("Shutting down tcpSocket!");
		shutdown(tcpSocket, SHUT_RDWR);
		close(tcpSocket);
		isConnected = false;
	}

	tcpSocket = 0;
}

void remoteLogInitialize(const char* loggingID, uint16_t port)
{
	remoteLogTerm();

	gLoggingID = std::string(loggingID);
	gPort      = port;
	tcpSocket  = 0;
	isConnected = false;
	isInitialized = true;

    listener = socket(AF_INET, SOCK_DGRAM, 0);
//	setNonBlocking(listener);

    struct sockaddr_in myaddr;
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(port);

	int err = bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr));
	if(err < 0)
		LOGE("Could not bind socket, %d %s", errno, strerror(err));

}

static bool connectToLogServer(BroadcastInfo info)
{
	struct sockaddr_in servaddr;
	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	setNonBlocking(tcpSocket);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(info.ip);
	servaddr.sin_port = htons(info.port);

	fd_set set;
	FD_ZERO(&set);
	FD_SET(tcpSocket, &set);

    struct timeval  timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

	auto tcperr = connect(tcpSocket, (struct sockaddr *)&servaddr, sizeof(servaddr));
	auto status = select(tcpSocket + 1, NULL, &set, NULL, &timeout);

	if(status < 0)
	{
		shutdownConnection();
		return false;
	}

	LOGE("Connected successfully!");

	setBlocking(tcpSocket);
	setsockopt(tcpSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
	uint16_t length = gLoggingID.size();
	send(tcpSocket, &length, sizeof(length), 0);
	send(tcpSocket, gLoggingID.c_str(), (uint32_t)length, 0);

	isConnected = true;
	return true;
}

static void listenForLogServer()
{
	BroadcastInfo info;
	struct sockaddr dummy;
	socklen_t len;

	auto read = recvfrom(listener, &info, sizeof(info), 0, &dummy, &len);
	LOGI("I listened for a server! %d", read);
	if(read == 6)
	{
		connectToLogServer(info);
	}
	else if(read < 0)
	{
		ASSERT(errno == EWOULDBLOCK || errno == EAGAIN, "Error in receiving logic");
	}
}

static void sendLogMessage(int verbosity, const char* toLog)
{
	LOGI("Sending message %s", toLog);
	LOGI("TCP SOCKET %d", tcpSocket);
	uint32_t data;
	auto err = send(tcpSocket, &verbosity, 1, 0);

	if(err < 0)
	{
		LOGI("There was an error during send 0");
		shutdownConnection();
	}

	//File and line information which is not present here.
	//Maby this should be done in the other application? It should.
	data = 0;
	err = send(tcpSocket, &data, 2, 0);

	if(err < 0)
	{
		LOGI("There was an error during send 1");
		shutdownConnection();
	}

	err = send(tcpSocket, &data, 4, 0);

	if(err < 0)
	{
		LOGI("There was an error during send 2");
		shutdownConnection();
	}

	data = strlen(toLog);
	err = send(tcpSocket, &data, 2, 0);

	if(err < 0)
	{
		LOGI("There was an error during send 3");
		shutdownConnection();
	}

	err = send(tcpSocket, toLog, data, 0);

	if(err < 0)
	{
		LOGI("There was an error during send 4");
		shutdownConnection();
	}
}

void remoteLogFormat(int verbosity, const char* fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	vsprintf(text_buffer, fmt, argptr);
	va_end(argptr);

	remoteLuaLog(verbosity, text_buffer);
}

void remoteLuaLog(int verbosity, const char* toLog)
{
	if(!isConnected && isInitialized)
		listenForLogServer();

	if(isConnected)
		sendLogMessage(verbosity, toLog);
	else
	{
		if(verbosity == 0)
			LOGI("%s", toLog);
		else if(verbosity == 1)
			LOGW("%s", toLog);
		else
			LOGE("%s", toLog);
	}
}

void remoteLogTerm()
{
	LOGE("Terminating logging connection");
	if(listener != 0)
	{
		close(listener);
	}

	shutdownConnection();

	isConnected = false;
	isInitialized = false;
	tcpSocket   = 0;
	listener    = 0;
	gPort 		= 0;
	gLoggingID.clear();
}

