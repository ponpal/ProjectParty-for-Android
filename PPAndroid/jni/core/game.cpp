/*
 * game.cpp
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#include "game.h"
#include "assert.h"
#include "stdlib.h"
#include <string>
#include "lua_core.h"
#include "lua_core_private.h"
#include <time.h>
#include "platform.h"
#include "unistd.h"
#include "async_operations.h"

#define __STDC_FORMAT_MACROS

#include <inttypes.h>

Game* gGame;
bool hasStarted = false;

bool gameInitialized() {
	return gGame != nullptr;
}

static void gameStart() {
	RLOGI("%s", "Starting game!");
	hasStarted = true;
	luaStartCall(gGame->L);
	nice(1000000);
}

static void gameRestart() {
	RLOGI("%s", "Restarting game!");
	luaRestartCall(gGame->L);
}

void gameInitialize(uint32_t screenWidth, uint32_t screenHeight) {
	if (gGame)
		return;
	Profile profile("Game initialize");

	remoteDebugStart(platformDeviceName());

	gGame = new Game();
	gGame->clock = new Clock();
	clockStart(gGame->clock);
	gGame->sensor = new SensorState();
	gGame->screen = new Screen();
	gGame->screen->width = screenWidth;
	gGame->screen->height = screenHeight;
	gGame->fps = 60;

	RLOGI("%s", "Initializing Game!");
	gGame->L = luaCoreCreate();
	if(hasStarted)
		gameRestart();
	else
		gameStart();

}

void gameStop() {
	if(!gGame)
		return;

	luaStopCall(gGame->L);
	luaCoreDestroy(gGame->L);
	asyncOperationsCancel();
	remoteDebugStop();
	delete gGame->clock;
	delete gGame->sensor;
	delete gGame;
	gGame = nullptr;
}

void gameTerminate() {
	hasStarted = false;
}

uint64_t target_frame = 0;
uint32_t fps = 0;
uint32_t frames = 0;
uint64_t next_second = 0;
int32_t sleep_offset = 0;
void gameStep(ndk_helper::GLContext* context) {
    clockStep(gGame->clock);

    remoteDebugUpdate();
    luaStepCall(gGame->L);
    asyncOperationsProcess();
    luaRunGarbageCollector(gGame->L, 3);

    struct timespec time1, time2;
    time1.tv_sec = 0;
    time1.tv_nsec = target_frame - sleep_offset - timeNowMonoliticNsecs();
    auto should_sleep = target_frame - timeNowMonoliticNsecs();
    auto before = timeNowMonoliticNsecs();
    nanosleep(&time1, &time2);
    auto after = timeNowMonoliticNsecs();
    frames++;
    context->Swap();
    target_frame = timeNowMonoliticNsecs() + 1000000000L/gGame->fps;

    if(next_second <= timeNowMonoliticNsecs())
    {
    	if(frames + 1 < gGame->fps)
    		sleep_offset += 250000;
    	else if(frames - 1 > gGame->fps)
    		sleep_offset -= 250000;
    	if(sleep_offset < 0)
    		sleep_offset = 0;
    	next_second = timeNowMonoliticNsecs() + 1000000000L;
    	frames = 0;
    }
}

void gameFinish() {
	gameStop();
    gameTerminate();
    platformExit();
}
