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

Game* gGame;
messageHandler gMessageHandler;

void gameInitialize(android_app* app)
{
	gGame = new Game();
	gGame->clock	 = new Clock();
	gGame->sensor 	 = new SensorState();
	gGame->screen    = new Screen();
	gGame->network	 = networkInitialize(app);
	gGame->renderer  = rendererInitialize(1024);

	LOGI("Initializing Game!");
	initializeLuaCore();
}

void gameTerminate()
{
	delete gGame->clock;
	delete gGame->sensor;

	networkDelete(gGame->network);
	rendererDelete(gGame->renderer);

	delete gGame;
}

void gameStart()
{
	LOGI("Starting game!");
	clockStart(gGame->clock);
	networkConnect(gGame->network);
}

void gameRestart()
{
	LOGI("Restarting game!");
	clockStart(gGame->clock);
	networkReconnect(gGame->network);
}

void gameResume()
{
	gGame->paused = false;
}

void gamePause()
{
	gGame->paused = true;
}

void gameSurfaceCreated()
{
	rendererActivate(gGame->renderer);
	initializeLuaScripts();
	initLuaCall();
}

void gameSurfaceDestroyed()
{
}

void gameStop()
{
	networkDisconnect(gGame->network);
//	rendererDeactivate(gGame->renderer);
	termLuaCall();
}

void handleFileTransfer(Buffer* buffer) {

	auto type = bufferReadByte(buffer);
	AutoPtr<char> path;
	auto stringSize = bufferReadUTF8(buffer, &path.ptr);
	auto fileSize = bufferReadLong(buffer);

	LOGI("Received a file! name=%s type=%d size=%llu", path.ptr, type, fileSize);

	//Open Asset here - or do something else that is nice.
	auto count = bufferBytesRemaining(buffer);
	auto externalsDir = gApp->activity->externalDataPath;
	std::string filePath(externalsDir);
	filePath += "/";
	for(int i = 0; i < stringSize; i++) {
		if(path.ptr[i] == '/') {
			int err = mkdir(filePath.c_str(), 0770);
			if(err != 0 && errno != 17)
				LOGI("Failed to make directory: %d %s", errno, strerror(errno));
		}
		filePath += path.ptr[i];
	}

	auto file = fopen(filePath.c_str(), "w+");
	if(file == NULL) {
		LOGE("Unable to write/create file: %s", filePath.c_str());
	} else {
		LOGI("Created file %s", filePath.c_str());
	}

	AutoPtr<uint8_t> chunckBuffer(0xFFFF);
	while (true) {
		auto toRead = fileSize < count ? fileSize : count;
		auto read = bufferReadBytes(buffer, chunckBuffer.ptr, toRead);
		fwrite(chunckBuffer.ptr, sizeof(uint8_t), read, file);
		fileSize -= read;
		if (fileSize == 0)
			break;

		count = networkReceive(gGame->network);
	}

	fclose(file);
	LOGI("SUCESSFULLY WROTE A FILE!");
}

void gameHandleReceive()
{
	auto count = networkReceive(gGame->network);
	if(count == 0) return;

	auto buffer = gGame->network->in_;
	size_t remaining;
	while(true)
	{
		if((remaining = bufferBytesRemaining(buffer)) == 0) {
			count = networkReceive(gGame->network);
			if(count == 0) return;
		}

		auto size = bufferReadShort(buffer);
		auto id   = bufferReadByte(buffer);

		if(id == NETWORK_FILE) {
			handleFileTransfer(buffer);
		} else {
		auto end = buffer->ptr + size - 1;
			if(gMessageHandler != NULL) {
				gMessageHandler(id, size - 1);
			}
			buffer->ptr = end; //In case the lua code did something wrong. It feels wrong to crash the application imho.
		}
	}
}

void gameStep()
{
	clockStep(gGame->clock);
	if(networkIsAlive(gGame->network)) {
		gameHandleReceive();
		runLuaGarbageCollector(1);
		updateLuaCall();
		renderLuaCall();
	}
}
