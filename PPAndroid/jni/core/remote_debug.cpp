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
#include "remote_debug.h"
#include <fcntl.h>
#include <cstdio>
#include "socket_helpers.h"
#include "service_finder.h"
#include "lua_core_private.h"
#include "game.h"

#define SERVICE_NAME "DEBUG_SERVICE"
#define QUERY_INTERVAL 500
#define CONNECTION_TYPE 1
#define CONNECTION_TIMEOUT 10000

#define SEND_TIMEOUT 1000
#define RECV_TIMEOUT 1
#define PORT 23451

#define VALID_LUA_RESULT 1
#define INVALID_LUA_RESULT 2

static std::string gLoggingID;
static int tcpSocket = 0;
static bool isConnected = false;
static pthread_mutex_t mutex;

static int csend(int socket, const void* data, size_t len, int opt)
{
	int res = send(socket, data, len, opt);
	if(res < 0)
	{
		LOGE("Error sending data! Error: %d %s", errno, strerror(errno));
	}
	return res;
}

static void onConnectResult(int socket, bool connected)
{
	if(connected)
	{
		socketSetBlocking(tcpSocket, true);
		socketTcpNoDelay(tcpSocket, true);
		socketSendTimeout(tcpSocket, SEND_TIMEOUT);
		socketRecvTimeout(tcpSocket, RECV_TIMEOUT);

		uint8_t connectionType = CONNECTION_TYPE;
		csend(tcpSocket, &connectionType, 1, 0);

		uint16_t length = gLoggingID.size();
		csend(tcpSocket, &length, sizeof(length), 0);
		csend(tcpSocket, gLoggingID.c_str(), (uint32_t)length, 0);

		isConnected = true;
		LOGI("Connected to the remote logging server!");
	}
	else
	{
		LOGE("Failed to connect to the debuging server!");
		LOGI("Restarting Remote Debugging!");
		remoteDebugStart(gLoggingID.c_str());
	}
}

static void onServiceFound(ServiceEvent* event, bool success)
{
	if(success)
	{
		auto buffer = event->buffer;
		auto ip = bufferReadInt(buffer);
		auto port = bufferReadShort(buffer);

		tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
		LOGI("Found Connection: IP=%X Port=%d", ip, (uint32_t)port);
		socketAsyncConnect(tcpSocket, ip, port, CONNECTION_TIMEOUT, &onConnectResult);
	}
	else
	{
		LOGI("Debug service found operation failed!");
		LOGI("Restarting Remote Debugging!");
		remoteDebugStart(gLoggingID.c_str());
	}
}

static bool sendLogMessage(int verbosity, const char* toLog)
{
	if(verbosity > 2)
	{
		LOGE("Invalid verbosity! %d", verbosity);
	}

	auto len = strlen(toLog) + 1;

	uint8_t logHeader = 0;
	auto err = csend(tcpSocket, &logHeader, 1, 0);
	if(err < 0) goto failure;

	err = csend(tcpSocket, &verbosity, 1, 0);
	if(err < 0)	goto failure;

	err = csend(tcpSocket, &len, 2, 0);
	if(err < 0) goto failure;

	err = csend(tcpSocket, toLog, len, 0);
	if(err < 0) goto failure;

	return true;

	failure:
	LOGE("There was an error sending a log message!");
	LOGE("Error was: %d %s", errno, strerror(errno));
	remoteDebugStart(gLoggingID.c_str());
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

static void sendConsoleResult(bool noError, const char* result)
{
	uint8_t typ = noError ? VALID_LUA_RESULT : INVALID_LUA_RESULT;
	uint16_t length = strlen(result) + 1; //We include the null terminator!

	auto err = csend(tcpSocket, &typ, 1, 0);
	if(err < 0) goto failure;

	err = csend(tcpSocket, &length, 2, 0);
	if(err < 0) goto failure;

	err = csend(tcpSocket, result, length, 0);
	if(err < 0) goto failure;

	return;
	failure:
	LOGE("There was an error sending a console message!");
	LOGE("Error was: %d %s", errno, strerror(errno));
	remoteDebugStart(gLoggingID.c_str());
}

void remoteDebugUpdate()
{
	if(!isConnected) return;

	//Read message: Do message and then Resend message.
	uint16_t length;
	int read = recv(tcpSocket, &length, 2, 0);
	if(read < 0)
	{
		if(errno != EWOULDBLOCK && errno != EAGAIN)
			goto failure;
	}
	else
	{
		uint8_t* data = (uint8_t*)alloca(length);
		int readData = 0;

		while(readData != length)
		{
			read = recv(tcpSocket, &data[readData], length - readData, 0);
			if(read < 0)
			{
				if(errno != EWOULDBLOCK && errno != EAGAIN)
					goto failure;
			}
			else
			{
				readData += read;
			}
		}

		LOGI("Received data: %s", (char*)data);

		char* res;
		auto err = luaConsoleInputCall(gGame->L, (char*)data, &res);
		if(res == nullptr)
			res = (char*)"No result";
		sendConsoleResult(err, res);
	}

	return;

	failure:
	LOGE("Remote connection failed. Error: %d %s", errno, strerror(errno));
	remoteDebugStart(gLoggingID.c_str());
}

static void remoteLogReset()
{
	if(tcpSocket != 0) {
		shutdown(tcpSocket, SHUT_RDWR);
		close(tcpSocket);
	}

	tcpSocket = 0;
	isConnected = false;
}

void remoteDebugStart(const char* loggingID)
{
	gLoggingID = loggingID;
	remoteLogReset();

	serviceFinderAsync(SERVICE_NAME, PORT, &onServiceFound, QUERY_INTERVAL);
}

void remoteDebugStop()
{
	LOGI("Remote logging terminated");
	remoteLogReset();
	gLoggingID.clear();
}

void remoteDebugInitialize()
{
	pthread_mutex_init(&mutex, 0);
}
