/*
 * lua_core_private.h
 *
 *  Created on: Feb 20, 2014
 *      Author: Lukas_2
 */

#ifndef LUA_CORE_PRIVATE_H_
#define LUA_CORE_PRIVATE_H_
#include <android_native_app_glue.h>
#include "JNIHelper.h"
#include "lua.hpp"
#include "buffer.h"


void luaHandleMessageCall(lua_State* L, Buffer* buffer, uint16_t id);

lua_State* luaCoreCreate();
void luaCoreDestroy(lua_State* L);

void luaStartCall(lua_State* L);
void luaRestartCall(lua_State* L);
void luaStopCall(lua_State* L);
void luaStepCall(lua_State* L);
void luaRunGarbageCollector(lua_State* L, int milisecs);

bool luaConsoleInputCall(lua_State* L, const char* input, char** result);

bool luaMenuCall(lua_State* L);
bool luaBackCall(lua_State* L);

void luaOnUpCall(lua_State* L, uint32_t pointerID, float x, float y);
void luaOnDownCall(lua_State* L, uint32_t pointerID, float x, float y);
void luaOnMoveCall(lua_State* L, uint32_t pointerID, float x, float y);
void luaOnCancelCall(lua_State* L, uint32_t pointerID, float x, float y);
void luaOnInputString(lua_State* L, const char* input);
void luaStackDump(lua_State* L);
#endif /* LUA_CORE_PRIVATE_H_ */



