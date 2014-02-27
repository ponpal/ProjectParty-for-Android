/*
 * game.h
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#ifndef GAME_H_
#define GAME_H_

#include "network.h"
#include "time_helpers.h"
#include "lua_core_private.h"
#include "sensor.h"
#include "GLContext.h"
#include "new_renderer.h"
#include "content.h"

extern "C"
{
	typedef struct
	{
		uint32_t width, height;
	} Screen;

	typedef struct
	{
		Clock* clock;
		Network* network;
		SensorState* sensor;
		Renderer* renderer;
		Screen* screen;
		Content* content;
		bool paused;

	} Game;

	typedef void (*messageHandler)(uint8_t id, uint32_t length);
	extern messageHandler gMessageHandler;
	extern Game* gGame;
}

void gameInitialize(android_app* app);
void gameStart();
void gameRestart();
void gameResume();
void gamePause();
void gameSurfaceCreated();
void gameSurfaceDestroyed();
void gameStop();

void gameTerminate();
void gameStep();

#endif /* GAME_H_ */
