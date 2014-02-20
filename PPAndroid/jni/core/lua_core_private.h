/*
 * lua_core_private.h
 *
 *  Created on: Feb 20, 2014
 *      Author: Lukas_2
 */

#ifndef LUA_CORE_PRIVATE_H_
#define LUA_CORE_PRIVATE_H_
#include <android_native_app_glue.h>
#include "NDKHelper.h"

void initializeLuaCore(android_app* app);
void initLuaCall();
void termLuaCall();
void updateLuaCall();
void renderLuaCall(ndk_helper::GLContext* context);

#endif /* LUA_CORE_PRIVATE_H_ */
