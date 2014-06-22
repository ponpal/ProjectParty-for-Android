/*
 * types.h
 *
 *  Created on: Feb 11, 2014
 *      Author: Lukas_2
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <glm/glm.hpp>

typedef struct { float x; float y; } vec2f;
typedef struct { float x; float y; float z; } vec3f;
typedef struct { float x; float y; float z; float w; } vec4f;

typedef struct { float mat[16]; } matrix4;

typedef struct {
	uint32_t glName;
	uint32_t width, height;
} Texture;

typedef struct {
	Texture texture;
	float x, y, width, height;
} Frame;

#endif /* TYPES_H_ */
