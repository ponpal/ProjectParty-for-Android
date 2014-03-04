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

void initializeLuaCore();
void initializeLuaScripts(const std::string& gameName);
void initializeRenderer();

void initLuaCall();
void termLuaCall();
void updateLuaCall();
void renderLuaCall();
void runLuaGarbageCollector(int milisecs);
void loadLuaScript(const std::string& relativeScriptPath);

#endif /* LUA_CORE_PRIVATE_H_ */
