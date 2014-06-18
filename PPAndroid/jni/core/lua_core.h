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
	void luaLog(const char* toLog);
	const char* bufferReadLuaString(Buffer* buffer);
    void callLuaHandleMessage(uint32_t id, uint32_t length);
}
#endif /* LUA_CORE_H_ */
