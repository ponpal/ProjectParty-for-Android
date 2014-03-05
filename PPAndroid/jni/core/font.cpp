/*
 * font.cpp
 *
 *  Created on: Feb 13, 2014
 *      Author: Lukas_2
 */

#include <glm/glm.hpp>
#include <cstring>
#include <cmath>
#include "font.h"
#include "external_libs/utf8.h"

namespace font
{
	glm::vec2 measure(const Font& font, const char* text)
	{
		float width = 0, height = 0, cursor = 0;

		auto spaceInfo = font[' '];
		auto itr   = &text[0];
		auto end   = &text[strlen(text)];

		while(itr != end)
		{
			auto c = utf8::next(itr, end);
			if(c == '\r') continue;

			if(c == ' ') {
				cursor += spaceInfo.advance;
				continue;
			} else if(c == '\n') {
				height += font.lineHeight;
				width  = fmaxf(cursor, width);
				cursor = 0;
				continue;
			} else if(c == '\t') {
				cursor += spaceInfo.advance * 4;
			}

			CharInfo info = font.chars[c];
			cursor += info.advance;
			height = fmax(height, -info.srcRect.w);
		}

		width = fmaxf(width, cursor);
		return glm::vec2(width, height);
	}
}
