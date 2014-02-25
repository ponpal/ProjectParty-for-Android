/*
 * game.h
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#ifndef GAME_H_
#define GAME_H_

#include "network.h"
#include "clock.h"
#include "lua_core_private.h"
#include "sensor.h"

extern "C"
{
	typedef struct
	{
		Clock* clock;
		Network* network;
		SensorState* sensor;
		//Loader* loader;
		//Renderer* renderer;
		bool paused;

	} Game;

	typedef void (*messageHandler)(uint8_t id, uint32_t length);
	extern messageHandler gMessageHandler;
	extern Game* gGame;
}

void gameInitialize(android_app* app);
void gameStart();
void gameStop();

void gameTerminate();
void gameStep(ndk_helper::GLContext* context);

#endif /* GAME_H_ */
