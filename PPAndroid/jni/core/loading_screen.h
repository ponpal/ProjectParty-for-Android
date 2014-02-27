/*
 * loading_screen.h
 *
 *  Created on: Feb 27, 2014
 *      Author: Gustav
 */

#ifndef LOADING_SCREEN_H_
#define LOADING_SCREEN_H_

#include "new_renderer.h"

#define LOADING_FRAME_PATH "loadingScreen.png"

class LoadingScreen
{
	Frame loadingFrame;

public:
	LoadingScreen();
	void load();
	void unload();
	void draw(Renderer* renderer, glm::vec2 dim);
};


#endif /* LOADING_SCREEN_H_ */
