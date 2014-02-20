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

struct Color
{
	uint32_t packedValue;

	Color() : packedValue(0xFFFFFFFF)
	{ }

	Color(uint32_t hex)
		: packedValue(hex)
	{ }
};

struct Texture
{
	GLuint glName;

	Texture()
	: glName(0xFFFFFFFF)
	{ }

	Texture(GLuint name)
	: glName(name)
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
