/*
 * game.h
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#ifndef GAME_H_
#define GAME_H_

#define ORIENTATION_LANDSCAPE ((uint8_t)0)
#define ORIENTATION_PORTRAIT  ((uint8_t)1)

#include "types.h"
#include "network.h"
#include "time_helpers.h"
#include "lua_core_private.h"
#include "GLContext.h"
#include "new_renderer.h"

extern "C"
{
    typedef struct
    {
        vec3f acceleration;
    } SensorState;

	typedef struct
	{
		uint32_t width, height;
		uint8_t orientation;
	} Screen;

	typedef struct
	{
		Clock* clock;
		Network* network;
		SensorState* sensor;
		Renderer* renderer;
		Screen* screen;
		bool paused;
		char* name;
		char* resourceDir;
		uint32_t fps;
	} Game;

	extern Game* gGame;
}

bool gameInitialized();

void gameInitialize();
void gameTerminate();
void gameStop();

void gameStep(ndk_helper::GLContext* context);
void gameFinish();

#endif /* GAME_H_ */
