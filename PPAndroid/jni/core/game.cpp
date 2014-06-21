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

Game* gGame;

bool hasStarted = false;

void (*resourcesChangedCallback)(void);

bool gameInitialized() {
	return gGame != nullptr;
}

static void gameStart() {
	LOGI("Starting game!");
	hasStarted = true;
	luaStartCall(gGame->L);
}

static void gameRestart() {
	LOGI("Restarting game!");
	luaRestartCall(gGame->L);
}

void gameInitialize() {
	if (gGame)
		return;

	gGame = new Game();
	gGame->clock = new Clock();
	clockStart(gGame->clock);
	gGame->sensor = new SensorState();
	gGame->screen = new Screen();
	gGame->fps = 60;

	resourcesChangedCallback = nullptr;

	LOGI("Initializing Game!");
	gGame->L = luaCoreCreate();
	if(hasStarted)
		gameRestart();
	else
		gameStart();
}

void gameStop() {
	if(!gGame)
		return;

	delete gGame->clock;
	delete gGame->sensor;
	delete gGame;
	gGame = nullptr;
}

void gameTerminate() {
	hasStarted = false;

}

void gameStep(ndk_helper::GLContext* context) {
    clockStep(gGame->clock);
    luaStepCall(gGame->L);
    context->Swap();
    auto time = gGame->clock->_lastTime;
    auto target = time + 1000000000L/gGame->fps - timeNowMonoliticNsecs();
    luaRunGarbageCollector(gGame->L, target/1000000);
    target = time + 1000000000L/gGame->fps - timeNowMonoliticNsecs();
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
    platformExit();
}
