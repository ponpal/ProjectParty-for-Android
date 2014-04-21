/*
 * lua_core.h
 *
 *  Created on: Feb 20, 2014
 *      Author: Lukas_2
 */

#ifndef LUA_CORE_H_
#define LUA_CORE_H_


#include "types.h"
#include "buffer.h"
using namespace glm;

extern "C"
{
	unsigned int loadFrame(const char* name);
	unsigned int loadFont(const char* fontName);
	void unloadFrame(uint32_t frame);
	void unloadFont(uint32_t font);

	vec2f measureString(unsigned int font, const char* str);
	void addFrame(unsigned int frame, vec2f pos, vec2f dim, unsigned int color);
	void addFrame2(unsigned int frame, vec2f pos, vec2f dim, unsigned int color,
			vec2f origin, float rotation, int mirrored);
	void addText(unsigned int font, const char* str, vec2f pos, unsigned int  color);

	void luaLog(const char* toLog);
	const char* bufferReadLuaString(Buffer* buffer);

	const char* testStr();
    void callLuaHandleMessage(uint32_t id, uint32_t length);

}
#endif /* LUA_CORE_H_ */
