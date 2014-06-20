/*
 * game.cpp
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#include "game.h"
#include "asset_helper.h"
#include "assert.h"
#include "stdlib.h"
#include <string>
#include "sys/stat.h"
#include "errno.h"
#include "path.h"
#include "lua_core.h"
#include "lua_core_private.h"
#include <time.h>
#include "unistd.h"
#include "new_renderer.h"
#include "pthread.h"
#include "file_manager.h"
#include "resource_manager.h"
#include "network.h"
#include "android_platform.h"
#include "server_discovery.h"

#define NUM_RESOURCES 256

Game* gGame;

bool hasStarted = false;

void (*resourcesChangedCallback)(void);

bool gameInitialized() {
	return gGame != nullptr;
}

static void gameStart() {
	LOGI("Starting game!");
	hasStarted = true;
	serverDiscoveryStart();
	if (networkConnect(gGame->network, platformGetIP(), 12345, platformGetPort()) == -1)
		gameFinish(); //To be replaced by reconnect screen eventually.
}

static void gameRestart() {
	LOGI("Restarting game!");
	auto result = networkReconnect(gGame->network);
	LOGE("RESULT: %d", result);
	if(result == -1 || result == SERVER_REFUSED_RECONNECT || result == COULD_NOT_RECONNECT)
		gameFinish();
}

void gameInitialize() {
	if (gGame)
		return;

	gGame = new Game();
	gGame->clock = new Clock();
	clockStart(gGame->clock);
	gGame->sensor = new SensorState();
	gGame->screen = new Screen();
	gGame->network = networkCreate(1024);
	gGame->renderer = rendererInitialize(1024);
	rendererActivate(gGame->renderer);
	gGame->fps = 60;
	gGame->resourceDir = nullptr;
	gGame->name = nullptr;

	resourcesChangedCallback = nullptr;

	LOGI("Initializing Game!");
	initializeLuaCore();
	if(hasStarted)
		gameRestart();
	else
		gameStart();
}

void gameStop() {
	if(!gGame)
		return;

	contentUnloadAll();
	contentTerminate();
	termLuaCall();
	networkDisconnect(gGame->network);
	networkDestroy(gGame->network);
	rendererDelete(gGame->renderer);
	if (gGame->name != nullptr)
        delete gGame->name;
	if (gGame->resourceDir != nullptr)
        delete gGame->resourceDir;
	delete gGame->clock;
	delete gGame->sensor;
	delete gGame;
	gGame = nullptr;
}

void gameTerminate() {
	hasStarted = false;
}

void handleAllResourcesLoaded() {
	LOGI("All resources loaded! :)");
	initializeLuaScripts();
	resourcesLoadedLuaCall();
}

void handleGameName()
{
    std::string resourceFolder = path::buildPath(gApp->activity->externalDataPath, gGame->name);
    char* resourceFolderC = new char[resourceFolder.size()+1];
    memcpy(resourceFolderC, resourceFolder.c_str(), resourceFolder.size()+1);
    gGame->resourceDir = resourceFolderC;
    LOGI("ResourceFolder: %s", resourceFolderC);
   	int err = mkdir(resourceFolderC, 0770);
	if (err != 0 && errno != 17)
		LOGI("Error is: %d %s", errno, strerror(errno));
	contentInitialize(NUM_RESOURCES, gGame->resourceDir);
}

static void resourcesLoaded()
{
	handleAllResourcesLoaded();
	resourcesChangedCallback = nullptr;
}

static void resourceLoadingDone()
{
	resourcesChangedCallback = &resourcesLoaded;
}

void handleSystemMessage(Buffer* buffer)
{
	auto id = bufferReadByte(buffer);
    LOGI("system messageID: %d", id);
    if (id == NETWORK_IN_SHUTDOWN) {
        LOGE("Shutting down");
        gameFinish();
    } else if (id == NETWORK_IN_SETUP_FILE_TRANSFER) {
        auto len = bufferReadUTF8(buffer, &gGame->name);
        auto port = bufferReadShort(buffer);
        auto ip = bufferReadInt(buffer);

    	handleGameName();
        ReceiveFileConfig serverConfig = { ip, port, gGame->resourceDir, &resourceLoadingDone };
        receiveFiles(serverConfig);
    }
}

//bool readMessage(Buffer* buffer) {
//	auto remaining = bufferBytesRemaining(buffer);
//	if (remaining == 0) {
//		return true;
//	} else if (remaining == 1) {
//		tempBufferLength = 1;
//		tempBuffer[0] = bufferReadByte(buffer);
//		return false;
//	}
//
//	auto size = bufferReadShort(buffer);
//	remaining = bufferBytesRemaining(buffer);
//	if (remaining < size) {
//		(*(uint16_t*) tempBuffer) = size;
//		bufferReadBytes(buffer, tempBuffer + 2, remaining);
//		//LOGI("Got a temporary message. Size: %d, Remaining: %d",
//		//		size, remaining);
//		tempBufferLength = remaining + 2;
//		return false;
//	}
//
//	auto id = bufferReadShort(buffer);
//	LOGE("MESSAGE GOT: %d", (uint32_t)id);
//
//	if (id == 0) {
//		handleSystemMessage(buffer);
//} else {
//		auto end = buffer->ptr + size - 1;
//		callLuaHandleMessage(id, size - 1);
//		buffer->ptr = end; //In case the lua code did something wrong. It feels wrong to crash the application imho.
//	}
//	return true;
//}

bool readMessage(Buffer* buffer) {
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
        callLuaHandleMessage(messageID, messageSize - 2);
	}

	uint32_t expected = remaining - (messageSize + 2);
	uint32_t actual = bufferBytesRemaining(buffer);

	ASSERTF(expected == actual,"Faulty message. Expected: %d, Actual: %d", expected, actual);
	return true;
}

void gameHandleReceive() {
    auto count = networkReceive(gGame->network);
    if (count == -1) return;
    while (readMessage(gGame->network->in_)) { }

    count = networkUnreliableReceive(gGame->network);
    if (count == -1) return;
    while (readMessage(gGame->network->uin)) { }
}

void gameStep(ndk_helper::GLContext* context) {
    clockStep(gGame->clock);
    gameHandleReceive();
    runLuaGarbageCollector(1);
    updateLuaCall();
    context->Swap();
    auto time = gGame->clock->_lastTime;
    auto target = time + 1000000000L/gGame->fps - timeNowMonoliticNsecs();
    struct timespec time1, time2;
    time1.tv_sec = 0;
    time1.tv_nsec = target;
    nanosleep(&time1, &time2);
    if(resourcesChangedCallback != nullptr)
    {
    	resourcesChangedCallback();
    }
}

void gameFinish() {
	gameStop();
    gameTerminate();
    ANativeActivity_finish(gApp->activity);
}
