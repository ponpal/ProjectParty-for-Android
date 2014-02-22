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


struct CharInfo
{
	glm::vec4 textureCoords;
	glm::vec4 srcRect;
	glm::vec2 offset;
	float  advance;

	CharInfo()
	: textureCoords(NAN, NAN, NAN, NAN),
	  srcRect(NAN,NAN,NAN,NAN),
	  offset(NAN,NAN),
	  advance(NAN) { }

	CharInfo(glm::vec4 texCoords,
			 glm::vec4 sRect,
			 glm::vec2 off,
			 float adv)
	: textureCoords(texCoords),
	  srcRect(sRect),
	  offset(off),
	  advance(adv) { }
};

struct Font
{
	float base;
	float size;
	float lineHeight;
	Frame page;
	CharInfo* chars;
	size_t charsLength;

	CharInfo defaultChar;

	Font() { }

	Font(float _base, float _size,
		 float _lineHeight,
		 Frame _page,
		 CharInfo* _chars,
		 size_t _charsLength)
	: base(_base),
	  size(_size),
	  lineHeight(_lineHeight),
	  page(_page),
	  chars(_chars),
	  charsLength(_charsLength)
	{
		defaultChar = chars[' '];
	}

	inline const CharInfo& operator[](size_t index) const
	{
		if(index > charsLength)
			return defaultChar;
		else
		{
			CharInfo& info = chars[index];

			if(isnanf(info.advance))
				return defaultChar;
			else
				return info;
		}
	}
};

namespace font
{
	glm::vec2 measure(const Font& f, const std::string& text);
}


#endif /* FONT_H_ */
