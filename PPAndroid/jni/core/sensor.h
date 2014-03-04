/*
 * sensor.h
 *
 *  Created on: Feb 24, 2014
 *      Author: Lukas_2
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include <glm/vec3.hpp>
#include <android_native_app_glue.h>

using namespace glm;

typedef void (*touchHandler) (int x, int y, int pointerIndex);
typedef void (*tapHandler) (int x, int y);

typedef struct
{
	vec3 acceleration;
	vec3 gyroscope;
	touchHandler onTouch;
	tapHandler onTap;
} SensorState;


const int32_t DOUBLE_TAP_TIMEOUT = 300 * 1000000;
const int32_t TAP_TIMEOUT = 180 * 1000000;
const int32_t DOUBLE_TAP_SLOP = 100;
const int32_t TOUCH_SLOP = 8;

enum
{
    MY_GESTURE_STATE_NONE = 0,
    MY_GESTURE_STATE_START = 1,
    MY_GESTURE_STATE_MOVE = 2,
    MY_GESTURE_STATE_END = 4,
    MY_GESTURE_STATE_ACTION = (MY_GESTURE_STATE_START | MY_GESTURE_STATE_END),
};

typedef int32_t GESTURE_STATE;

typedef struct
{
	int32_t pointerID;
	int32_t x, y;
} TapDetector;

GESTURE_STATE tapDetect(TapDetector* detector, const AInputEvent* motion_event);


#endif /* SENSOR_H_ */
