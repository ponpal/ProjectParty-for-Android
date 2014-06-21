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
    void initializeLuaScripts(const char* scriptsDir);
}
#endif /* LUA_CORE_H_ */
