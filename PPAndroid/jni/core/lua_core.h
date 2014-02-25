/*
 * lua_core.h
 *
 *  Created on: Feb 20, 2014
 *      Author: Lukas_2
 */

#ifndef LUA_CORE_H_
#define LUA_CORE_H_


#include "types.h"
using namespace glm;

extern "C"
{
	unsigned int loadFrame(const char* name);
	unsigned int loadFont(const char* frameName, const char* fontName);
	void unloadFrame(unsigned int frame);
	void unloadFont(unsigned int font);

	vec2f measureString(unsigned int font, const char* str);
	void addFrame(unsigned int frame, vec2f pos, vec2f dim, unsigned int color);
	void addFrame2(unsigned int frame, vec2f pos, vec2f dim, unsigned int color,
			vec2f origin, float rotation, int mirrored);
	void addText(unsigned int font, const char* str, vec2f pos, unsigned int  color);

	void luaLog(const char* toLog);
}
#endif /* LUA_CORE_H_ */
