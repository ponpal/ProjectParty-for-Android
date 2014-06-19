/*
 * game.cpp
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#include "game.h"
#include "asset_helper.h"
#include "stdlib.h"
#include <string>
#include "sys/stat.h"
#include "errno.h"
#include "path.h"
#include "lua_core.h"
#include <time.h>
#include "unistd.h"
#include "new_renderer.h"
#include "pthread.h"
#include "file_manager.h"
#include <atomic>
#include "resource_manager.h"

#define NUM_RESOURCES 256

Game* gGame;
messageHandler gMessageHandler;

uint8_t tempBuffer[0xffff];
uint32_t tempBufferLength = 0;

void (*resourcesChangedCallback)(void);

void gameInitialize(android_app* app) {
	gGame = new Game();
	gGame->clock = new Clock();
	gGame->sensor = new SensorState();
	gGame->screen = new Screen();
	gGame->network = networkInitialize(app);
	gGame->renderer = rendererInitialize(1024);
	gGame->fps = 60;

	resourcesChangedCallback = nullptr;

	LOGI("Initializing Game!");
	initializeLuaCore();
}

void gameTerminate() {
	delete gGame->clock;
	delete gGame->sensor;
	if (gGame->name != nullptr)
        delete gGame->name;

	networkDelete(gGame->network);
	rendererDelete(gGame->renderer);
	contentTerminate();

	delete gGame;
}

void gameStart() {
	LOGI("Starting game!");
	clockStart(gGame->clock);
	if (networkConnect(gGame->network) == -1)
		gameFinish(); //To be replaced by reconnect screen eventually.
}

void gameRestart() {
	LOGI("Restarting game!");
	clockStart(gGame->clock);
	auto result = networkReconnect(gGame->network);
	LOGE("RESULT: %d", result);
	if(result == -1 || result == SERVER_REFUSED_RECONNECT || result == COULD_NOT_RECONNECT)
		gameFinish();
}

void gameResume() {
	gGame->paused = false;
}

void gamePause() {
	gGame->paused = true;
}

void gameSurfaceCreated() {
	rendererActivate(gGame->renderer);
}

void gameSurfaceDestroyed() {
	contentUnloadAll();
}

void gameStop() {
	networkDisconnect(gGame->network);
	contentUnloadAll();
	termLuaCall();
}

void handleAllResourcesLoaded() {
	LOGI("All resources loaded! :)");
	initializeLuaScripts();
	initLuaCall();
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
    if (id == NETWORK_SHUTDOWN) {
        LOGE("Shutting down");
        gameFinish();
    } else if (id == NETWORK_SETUP_FILE_TRANSFER) {
        auto len = bufferReadUTF8(buffer, &gGame->name);
        auto port = bufferReadShort(buffer);
        auto ip = bufferReadInt(buffer);

    	handleGameName();
        ReceiveFileConfig serverConfig = { ip, port, gGame->resourceDir, &resourceLoadingDone };
        receiveFiles(serverConfig);
    }
}

bool readMessage(Buffer* buffer) {
	auto remaining = bufferBytesRemaining(buffer);
	if (remaining == 0) {
		return true;
	} else if (remaining == 1) {
		tempBufferLength = 1;
		tempBuffer[0] = bufferReadByte(buffer);
		return false;
	}

	auto size = bufferReadShort(buffer);
	remaining = bufferBytesRemaining(buffer);
	if (remaining < size) {
		(*(uint16_t*) tempBuffer) = size;
		bufferReadBytes(buffer, tempBuffer + 2, remaining);
		//LOGI("Got a temporary message. Size: %d, Remaining: %d",
		//		size, remaining);
		tempBufferLength = remaining + 2;
		return false;
	}

	auto id = bufferReadShort(buffer);
	LOGE("MESSAGE GOT: %d", (uint32_t)id);

	if (id == 0) {
		handleSystemMessage(buffer);
} else {
		auto end = buffer->ptr + size - 1;
		callLuaHandleMessage(id, size - 1);
		buffer->ptr = end; //In case the lua code did something wrong. It feels wrong to crash the application imho.
	}
	return true;
}

void gameHandleReceive() {
	while (true) {
		auto count = networkReceive(gGame->network, tempBuffer,
				tempBufferLength);
		if (count == -1)
			return;

			tempBufferLength = 0;
			auto buffer = gGame->network->in_;
			while (readMessage(buffer)) {
				auto remaining = bufferBytesRemaining(buffer);
				if (remaining == 0)
					return;
			}
		}
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
    networkShutdown(gGame->network);
    ANativeActivity_finish(gApp->activity);
}
