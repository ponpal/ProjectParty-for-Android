/*
 * game.cpp
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#include "game.h"

Game* gGame;

void gameInitialize(android_app* app)
{
	gGame = new Game();
	gGame->clock = new Clock();
	gGame->network = networkInitialize(app);

	LOGI("Initializing Game!");
}

void gameTerminate()
{
	delete gGame->clock;
	delete gGame->network;
	delete gGame;
}

void gameStart()
{
	LOGI("Staring game!");

	initializeLuaCore();
	initLuaCall();
	clockStart(gGame->clock);
}

void gameStep(ndk_helper::GLContext* context)
{
	clockStep(gGame->clock);
	updateLuaCall();
	renderLuaCall(context);
}
