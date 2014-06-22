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
#include "lua.hpp"
using namespace glm;

extern "C"
{
    void luaLog(const char* toLog);
    void initializeLuaScripts(const char* scriptsDir);
    void loadLuaScripts(lua_State* L, const char* scriptsDirectory);
}
#endif /* LUA_CORE_H_ */
