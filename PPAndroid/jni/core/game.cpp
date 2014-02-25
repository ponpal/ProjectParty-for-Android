/*
 * game.cpp
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#include "game.h"
#include "asset_helper.h"

Game* gGame;
messageHandler gMessageHandler;

void gameInitialize(android_app* app)
{
	gGame = new Game();
	gGame->clock = new Clock();
	gGame->sensor = new SensorState();
	gGame->network = networkInitialize(app);

	LOGI("Initializing Game!");

	initializeLuaCore();
}

void gameTerminate()
{
	delete gGame->clock;
	delete gGame->network;
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
	initLuaCall();
}

void gameSurfaceDestroyed()
{
	termLuaCall();
}

void gameStop()
{
	networkDisconnect(gGame->network);
}

void handleFileTransfer(Buffer* buffer) {
	auto type = bufferReadByte(buffer);
	AutoPtr<char> path;
	auto stringSize = bufferReadUTF8(buffer, &path.ptr);
	auto fileSize = bufferReadLong(buffer);

	LOGI("Received a file! name=%s type=%d size=%llu", path.ptr, type, fileSize);

	//Open Asset here - or do something else that is nice.
	AutoPtr < uint8_t > asset;
	asset.ptr = new uint8_t[0xFFFF];

	auto count = bufferBytesRemaining(buffer);
	while (true) {
		auto toRead = fileSize < count ? fileSize : count;
		auto read = bufferReadBytes(buffer, asset.ptr, toRead);
		fileSize -= read;
		if (fileSize == 0)
			break;

		count = networkReceive(gGame->network);
	}


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
			LOGI("Count is %d", count);
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

void gameStep(ndk_helper::GLContext* context)
{
	clockStep(gGame->clock);
	if(networkIsAlive(gGame->network))
		gameHandleReceive();

	updateLuaCall();
	renderLuaCall(context);
}
