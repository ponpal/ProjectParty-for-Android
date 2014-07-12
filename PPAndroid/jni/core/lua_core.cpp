#include "lua_core.h"
#include "lua_core_private.h"
#include <cstring>
#include "game.h"
#include "JNIHelper.h"

#include <vector>
#include "path.h"
#include "platform.h"
#include "dirent.h"
#include "lua.hpp"
#include "remote_log.h"

lua_State* luaCoreCreate()
{
	auto luaState = luaL_newstate();
	luaL_openlibs(luaState);

	Resource coreScript = platformLoadInternalResource("core.lua");
	auto error = luaL_loadbuffer(luaState, (const char*)coreScript.buffer, coreScript.length, "core.lua");
	error = error | lua_pcall(luaState, 0,0,0);

	if(error)
		RLOGE("LUA CORE ERROR %s", lua_tostring(luaState, -1));

	platformUnloadResource(coreScript);

	return luaState;
}

void luaCoreDestroy(lua_State* L)
{
	lua_close(L);
}

static void loadLuaScript(lua_State* L, const std::string& pathInFiles)
{
	Resource script = platformLoadExternalResource(pathInFiles.c_str());

	int error = luaL_loadbuffer(L, (const char*)script.buffer, script.length, pathInFiles.c_str());
	error |= lua_pcall(L, 0, 0, 0);

	if(error)
		RLOGE("LUA SCRIPT ERROR %s in file %s", lua_tostring(L, -1), pathInFiles.c_str());

	platformUnloadResource(script);
}

void loadLuaScripts(lua_State* L, const char* scriptsDirectory)
{
	RLOGI("Entered initializeLuaScript: %s", scriptsDirectory);
	std::string scriptsDir = path::buildPath(platformExternalResourceDirectory(), scriptsDirectory);
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir (scriptsDir.c_str())) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if(path::hasExtension(ent->d_name, ".lua")) {
				std::string scriptPath = path::buildPath(scriptsDirectory, ent->d_name);
				loadLuaScript(L, scriptPath);
				RLOGI("Loading script %s", scriptPath.c_str());
			}
		}
		closedir(dir);
	} else {
		RLOGE("Couldn't open directory %s", scriptsDir.c_str());
		platformExit();
	}
	RLOGI("%s", "About to exit initializeLuaScript");
}

void luaRunGarbageCollector(lua_State* L, int milisecs)
{
	uint64_t nsecs = milisecs * 1000000L;

	auto now = timeNowMonoliticNsecs();
	while(timeNowMonoliticNsecs() - now < nsecs)
		if(lua_gc(L, LUA_GCSTEP, 0)) break;
}

static void callEmptyLuaFunction(lua_State* L, const char* buffer)
{
	int error = luaL_loadbuffer(L, buffer, strlen(buffer), "empty");
	error = error | lua_pcall(L, 0, 0, 0);
	if(error) {
		RLOGW("LUA Execution Error while calling %s. %s", buffer, lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static bool callEmptyLuaFunctionBool(lua_State* L, const char* buffer)
{
	lua_getglobal(L, buffer);
	int error = error | lua_pcall(L, 0, 1, 0);
	if(error) {
		RLOGW("LUA Execution Error while calling %s. %s", buffer, lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}
	/*retrieve result */
	if (!lua_isboolean(L, -1)) {
	 	RLOGW("%s", "function `f' must return a boolean");
	 	return false;
	}
	auto result = lua_toboolean(L, -1);
	lua_pop(L, 1);  /* pop returned value */

	return result;
}

static void callIntFloat2LuaFunction(lua_State* L, const char* methodName, uint32_t i, float x, float y)
{
	char buffer[128];
	sprintf(buffer, "%s(%d,%f,%f)", methodName, i, x, y);

	int error = luaL_loadbuffer(L, buffer, strlen(buffer), "int float float");
	error = error | lua_pcall(L, 0, 0, 0);

	if(error) {
		RLOGW("LUA Execution Error while calling %s. %s", methodName, lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

void luaStopCall(lua_State* L)
{
	callEmptyLuaFunction(L, "Game.stop()");
}

void luaStepCall(lua_State* L)
{
	callEmptyLuaFunction(L, "Game.step()");
}

void luaStartCall(lua_State* L)
{
	callEmptyLuaFunction(L, "Game.start()");
}

void luaRestartCall(lua_State* L)
{
	callEmptyLuaFunction(L, "Game.restart()");
}

void luaHandleMessageCall(lua_State* L, Buffer* buffer, uint32_t id)
{
	lua_getglobal(L, "handleMessage");
	lua_pushlightuserdata(L, buffer);
	lua_pushnumber(L, id);
	auto err = lua_pcall(L, 2, 0, 0);
	if(err) {
		RLOGW("LUA Execution Error %s", lua_tostring(L, -1));
	}
}

bool luaMenuCall(lua_State* L)
{
	return callEmptyLuaFunctionBool(L, "onMenuButton");
}

bool luaBackCall(lua_State* L)
{
	return callEmptyLuaFunctionBool(L, "onBackButton");
}

void luaOnUpCall(lua_State* L, uint32_t pointerID, float x, float y)
{
	callIntFloat2LuaFunction(L, "RawInput.onUp", pointerID, x, y);
}

void luaOnDownCall(lua_State* L, uint32_t pointerID, float x, float y)
{
	callIntFloat2LuaFunction(L, "RawInput.onDown", pointerID, x, y);
}

void luaOnMoveCall(lua_State* L, uint32_t pointerID, float x, float y)
{
	callIntFloat2LuaFunction(L, "RawInput.onMove", pointerID, x, y);
}

void luaOnCancelCall(lua_State* L, uint32_t pointerID, float x, float y)
{
	callIntFloat2LuaFunction(L, "RawInput.onCancel", pointerID, x, y);
}

