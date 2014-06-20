/*
 * main.h
 *
 *  Created on: Feb 23, 2014
 *      Author: Lukas_2
 */

#ifndef MAIN_H_
#define MAIN_H_
#include "NDKHelper.h"
#include "JNIHelper.h"
#include "core/display.h"
#include <android/sensor.h>
#include "core/game.h"
#include "android_native_app_glue.h"
#include "core/android_helper.h"

namespace lifecycle
{
	void create();
	void restart();
	void freshStart();
	void start();
	void resume();
	void pause();
	void stop();
	void destroy();
	void gainedFocus();
	void lostFocus();
	void surfaceCreated();
	void surfaceDestroyed();
	void surfaceChanged();
}


#endif /* MAIN_H_ */
