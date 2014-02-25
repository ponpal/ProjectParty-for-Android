/*
 * sensor.h
 *
 *  Created on: Feb 24, 2014
 *      Author: Lukas_2
 */

#ifndef SENSOR_H_
#define SENSOR_H_

typedef struct { float x, y, z; } vec3;

typedef struct
{
	vec3 acceleration;
	vec3 gyroscope;
} SensorState;

#endif /* SENSOR_H_ */
