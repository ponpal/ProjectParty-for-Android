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
#include "hash.h"
#include "remote_debug.h"

const CharInfo* fontCharInfo(const Font* font, size_t index)
{
	if(index > font->charsLength)
			return &font->chars[font->defaultChar];
    else
    {
        CharInfo* info = &font->chars[index];

        if(isnanf(info->advance))
            return &font->chars[font->defaultChar];
        else
            return info;
    }
}

Font* fontAtlasFindFont(FontAtlas* atlas, const char* fontName)
{
	HashID hash = bytesHash(fontName, strlen(fontName), 0);
	for(int i = 0; i < atlas->fontsLength ;i++)
	{
		if(atlas->fonts[i].hashID == hash)
			return &atlas->fonts[i];
	}

	RLOGI("Failed to find font! %s", fontName);
	return nullptr;
}

vec2f fontMeasure(const Font* font, const char* text)
{
    float width = 0, height = 0, cursor = 0;

    auto spaceInfo = fontCharInfo(font, ' ');
    auto itr   = &text[0];
    auto end   = &text[strlen(text)];

    while(itr != end)
    {
        auto c = utf8::next(itr, end);
        if(c == '\r') continue;

        if(c == ' ') {
            cursor += spaceInfo->advance;
            continue;
        } else if(c == '\n') {
            height += font->lineHeight;
            width  = fmaxf(cursor, width);
            cursor = 0;
            continue;
        } else if(c == '\t') {
            cursor += spaceInfo->advance * 4;
        }

        CharInfo info = font->chars[c];
        cursor += info.advance;
        height = fmax(height, -info.srcRect.w);
    }

    width = fmaxf(width, cursor);
    return (vec2f){width, height};
}
