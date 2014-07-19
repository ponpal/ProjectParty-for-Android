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
#include "strings.h"
#include "assert.h"
#include "platform.h"
#include <sys/time.h>
#include "remote_log.h"
#include <fcntl.h>
#include <cstdio>
#include "socket_helpers.h"
#include "service_finder.h"

#define SERVICE_NAME "LOGGING_SERVICE"
#define CONNECTION_TIMEOUT 1000
#define SEND_TIMEOUT 1000

static std::string gLoggingID;

static ServiceFinder* finder = nullptr;
static int tcpSocket = 0;
static bool isConnected = false, isInitialized = false;
static pthread_mutex_t mutex;

static void onServiceFound(const char* service, Buffer* buffer)
{
	auto ip = bufferReadInt(buffer);
	auto port = bufferReadShort(buffer);

	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(socketConnect(tcpSocket, ip, port, CONNECTION_TIMEOUT))
	{
		socketSetBlocking(tcpSocket, true);
		socketRecvTimeout(tcpSocket, SEND_TIMEOUT);

		uint16_t length = gLoggingID.size();
		send(tcpSocket, &length, sizeof(length), 0);
		send(tcpSocket, gLoggingID.c_str(), (uint32_t)length, 0);

		isConnected = true;
	}
	else
	{
		remoteLogStart(gLoggingID.c_str());
	}
}

static bool sendLogMessage(int verbosity, const char* toLog)
{
	if(verbosity > 2)
	{
		LOGE("Invalid verbosity! %d", verbosity);
	}

	auto len = strlen(toLog);
	auto err = send(tcpSocket, &verbosity, 1, 0);
	if(err < 0)	goto failure;

	err = send(tcpSocket, &len, 2, 0);
	if(err < 0) goto failure;

	err = send(tcpSocket, toLog, len, 0);
	if(err < 0) goto failure;

	return true;

	failure:
	LOGE("There was an error sending a log message!");
	LOGE("Error was: %d %s", errno, strerror(errno));
	remoteLogStart(gLoggingID.c_str());
	return false;
}

void remoteLogFormat(int verbosity, const char* fmt, ...)
{
	char text_buffer[2048];

	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(text_buffer, 2048, fmt, argptr);
	va_end(argptr);
	remoteLog(verbosity, text_buffer);
}

void remoteLog(int verbosity, const char* toLog)
{
	bool shouldLogWithPlatform = false;

	pthread_mutex_lock(&mutex);

	if(isInitialized && !isConnected)
	{
		if(!serviceFinderPollFound(finder))
			serviceFinderQuery(finder);
	}

	if(isConnected)
	{
		bool result = sendLogMessage(verbosity, toLog);
		shouldLogWithPlatform = !result;
	}
	else
	{
		shouldLogWithPlatform = true;
	}

	pthread_mutex_unlock(&mutex);

	if(shouldLogWithPlatform)
	{
		if(verbosity == 0)
			LOGI("%s", toLog);
		else if(verbosity == 1)
			LOGW("%s", toLog);
		else
			LOGE("%s", toLog);
	}
}

static void remoteLogReset()
{
	if(tcpSocket != 0) {
		shutdown(tcpSocket, SHUT_RDWR);
		close(tcpSocket);
	}

	if(finder != 0)
	{
		serviceFinderDestroy(finder);
	}

	finder = 0;
	tcpSocket = 0;
	isConnected = false;
	isInitialized = false;
}

void remoteLogStart(const char* loggingID)
{
	gLoggingID = loggingID;
	remoteLogReset();
	finder = serviceFinderCreate(SERVICE_NAME, servicePort, &onServiceFound);
   	isInitialized = true;

	if(!serviceFinderPollFound(finder))
		serviceFinderQuery(finder);
}

void remoteLogStop()
{
	LOGI("Remote logging terminated");
	remoteLogReset();
	gLoggingID.clear();
}

void remoteLogInitialize()
{
	pthread_mutex_init(&mutex, 0);
}

