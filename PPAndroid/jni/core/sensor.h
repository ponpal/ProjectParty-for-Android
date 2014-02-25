/*
 * sensor.h
 *
 *  Created on: Feb 24, 2014
 *      Author: Lukas_2
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include <glm/vec3.hpp>

using namespace glm;

typedef struct
{
	vec3 acceleration;
	vec3 gyroscope;
} SensorState;

#endif /* SENSOR_H_ */
