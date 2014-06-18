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
#include "loading_screen.h"
#include "path.h"
#include "lua_core.h"
#include <time.h>
#include "unistd.h"
#include "resource_manager.h"
#include "new_renderer.h"

Game* gGame;
messageHandler gMessageHandler;
bool hasLoadedResources;
LoadingScreen loadingScreen;
uint8_t tempBuffer[0xffff];
uint32_t tempBufferLength = 0;

void gameInitialize(android_app* app) {
	gGame = new Game();
	gGame->clock = new Clock();
	gGame->sensor = new SensorState();
	gGame->screen = new Screen();
	gGame->network = networkInitialize(app);
	gGame->renderer = rendererInitialize(1024);
	gGame->fps = 60;

	loadingScreen = LoadingScreen();
	LOGI("Herpa Derp");
	LOGI("Initializing Game!");
	initializeLuaCore();
	LOGI("Lua initialized!");

	LOGE("GameName: %d", (uint32_t)MESSAGE("GameName"));
	LOGE("GameName: %d", (uint32_t)shortHash("GameName", 8, 0));
}

void gameTerminate() {
	delete gGame->clock;
	delete gGame->sensor;
	if (gGame->name != nullptr)
        delete gGame->name;

	networkDelete(gGame->network);
	rendererDelete(gGame->renderer);

	delete gGame;
}

void gameStart() {
	hasLoadedResources = false;
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
	if (!hasLoadedResources)
		loadingScreen.load();

}

void gameSurfaceDestroyed() {
	contentUnloadAll();
}

void gameStop() {
	networkDisconnect(gGame->network);
	contentUnloadAll();
	termLuaCall();
}

void handleFileTransfer(Buffer* buffer) {
	AutoPtr<char> path;
	auto stringSize = bufferReadUTF8(buffer, &path.ptr);
	auto fileSize = bufferReadLong(buffer);

	LOGI("Received a file! name=%s size=%llu", path.ptr, fileSize);

	auto filePath = path::buildPath(gGame->resourceDir, path.ptr);

	auto file = fopen(filePath.c_str(), "w+");
	if (file == NULL) {
		LOGE("Unable to write/create file: %s", filePath.c_str());
	} else {
		LOGI("Created file %s", filePath.c_str());
	}

	auto count = bufferBytesRemaining(buffer);
	LOGI("Count: %d, Filesize: %d", count, (int)fileSize);
	AutoPtr<uint8_t> chunckBuffer(0xFFFF);
	while (true) {
		uint32_t toRead = fileSize < count ? fileSize : count;
		auto read = bufferReadBytes(buffer, chunckBuffer.ptr, toRead);

		fwrite(chunckBuffer.ptr, sizeof(uint8_t), read, file);
		fileSize -= read;
		if (fileSize == 0)
			break;

		count = networkReceive(gGame->network, tempBuffer, 0);
		if (count != 0)
			LOGI("count %d, filesize %d", count, (int)fileSize);
	}

	fclose(file);
	LOGI("SUCESSFULLY WROTE A FILE!");
	LOGI("Remaining: %d", bufferBytesRemaining(buffer));
}

void handleAllResourcesLoaded(Buffer* buffer) {
	hasLoadedResources = true;
	loadingScreen.unload();

	LOGI("Calling luaInit handleResources");
	initializeLuaScripts();
	initLuaCall();
}

void handleGameName(Buffer* buffer)
{
	bufferReadUTF8(buffer, &gGame->name);
	std::string path(gGame->name);
	std::string externalDir(gApp->activity->externalDataPath);
	std::string fullPath = path::buildPath(externalDir, gGame->name);
	char* mut = new char[fullPath.size() + 1];
	memcpy(mut, fullPath.c_str(), fullPath.size()+1);
	gGame->resourceDir = mut;
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

	if (id == MESSAGE("FileHeader")) {
		LOGI("Transfer");
		handleFileTransfer(buffer);
	} else if (id == MESSAGE("AllFilesSent")) {
		handleAllResourcesLoaded(buffer);
	} else if (id == MESSAGE("Shutdown")) {
		LOGE("Shutting down");
		gameFinish();
		return false;
	} else if (id == MESSAGE("GameName")) {
		LOGE("Game name");
		handleGameName(buffer);
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

//
//	auto count = networkReceive(gGame->network, tempBuffer, tempBufferLength);
//	if(count == 0)
//		return;
//	else if(count == -1)
//	{
//		gameFinish();
//		return;
//	}
//
//	auto buffer = gGame->network->in_;
//	size_t remaining;
//	while(true)
//	{
//		if((remaining = bufferBytesRemaining(buffer)) == 0) {
//			return;
//		}
//		LOGI("Bytes remaining: %d", bufferBytesRemaining(buffer));
//
//		auto size = bufferReadShort(buffer);
//
//		if (size > bufferBytesRemaining(buffer)) {
//			tempBufferLength = bufferBytesRemaining(buffer);
//			(*((uint16_t*)tempBuffer)) = size;
//			bufferReadBytes(buffer, tempBuffer + 2, tempBufferLength);
//			return;
//		}
//
//		auto id  = bufferReadByte(buffer);
//
//		LOGI("Received a message SIZE: %d, ID: %d", size, id);
//		if(id == NETWORK_FILE) {
//			handleFileTransfer(buffer);
//		} else if (id == NETWORK_ALLFILES) {
//			handleAllResourcesLoaded(buffer);
//		} else if (id == NETWORK_FILERELOAD) {
//			handleFileReload(buffer, size);
//		} else {
//			LOGI("Handling LUA message");
//            auto end = buffer->ptr + size - 1;
//			LOGI("Bytes remaining: %d", bufferBytesRemaining(buffer));
//            callLuaHandleMessage(id, size - 1);
//			LOGI("Bufpointer: %d", (size_t) buffer->ptr);
//			LOGI("End: %d", (size_t) end);
//			buffer->ptr = end; //In case the lua code did something wrong. It feels wrong to crash the application imho.
//			LOGI("Bytes remaining: %d", bufferBytesRemaining(buffer));
//		}
//	}
//	tempBufferLength = 0;

	void gameStep(ndk_helper::GLContext* context) {
		clockStep(gGame->clock);
		if (hasLoadedResources) {
			gameHandleReceive();
			runLuaGarbageCollector(1);
			updateLuaCall();
			context->Swap();
		} else {
			loadingScreen.draw(gGame->renderer,
					glm::vec2(gGame->screen->width, gGame->screen->height));
			context->Swap();
			gameHandleReceive();
		}
		auto time = gGame->clock->_lastTime;
		auto target = time + 1000000000L/gGame->fps - timeNowMonoliticNsecs();
		struct timespec time1, time2;
		time1.tv_sec = 0;
		time1.tv_nsec = target;
		nanosleep(&time1, &time2);
	}

	void gameFinish() {
		networkShutdown(gGame->network);
		ANativeActivity_finish(gApp->activity);
	}

