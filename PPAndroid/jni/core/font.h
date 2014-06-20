/*
 * font.h
 *
 *  Created on: Feb 13, 2014
 *      Author: Lukas_2
 */

#ifndef FONT_H_
#define FONT_H_

#include <glm/glm.hpp>
#include "types.h"
#include <string>

extern "C" {

typedef struct
{
	vec4f textureCoords;
	vec4f srcRect;
	vec2f offset;
	float  advance;
} CharInfo;

typedef struct
{
	Texture page;
	CharInfo* chars;
	size_t charsLength;
	size_t defaultChar;
	float size;
	float base;
	float lineHeight;
} Font;

const CharInfo* fontCharInfo(const Font* font, size_t index);


vec2f fontMeasure(const Font* f, const char* text);

}

#endif /* FONT_H_ */
