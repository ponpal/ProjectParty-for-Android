/*
 * lua_core.h
 *
 *  Created on: Feb 20, 2014
 *      Author: Lukas_2
 */

#ifndef LUA_CORE_H_
#define LUA_CORE_H_

extern "C"
{
	typedef struct { float x; float y; } vec2;

	unsigned int loadFrame(const char* name);
	unsigned int loadFont(const char* frameName, const char* fontName);
	void unloadFrame(unsigned int frame);
	void unloadFont(unsigned int font);

	vec2 measureString(unsigned int font, const char* str);

	void addFrame(unsigned int frame, vec2 pos, vec2 dim, unsigned int color);
	void addFrame2(unsigned int frame, vec2 pos, vec2 dim, unsigned int color,
				  vec2 origin, float rotation, int mirrored);
	void addText(unsigned int font, const char* str, vec2 pos, unsigned int  color);

	void luaLog(const char* toLog);
}
#endif /* LUA_CORE_H_ */
