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

struct Texture
{
	GLuint glName;
	uint16_t width, height;

	Texture()
	: glName(0xFFFFFFFF),
	  width(0), height(0)
	{ }

	Texture(GLuint name, uint16_t w, uint16_t h)
	: glName(name), width(w), height(h)
	{ }
};

struct Frame
{
	Texture texture;
	glm::vec4 coords;

	Frame() { }
	Frame(Texture tex,
		  glm::vec4 _coords)
	: texture(tex),
	  coords(_coords)
	{
	}
};

#endif /* TYPES_H_ */
