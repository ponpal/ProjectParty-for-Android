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

lua_State* luaCoreCreate()
{
	auto luaState = luaL_newstate();
	luaL_openlibs(luaState);

	Resource glScript = platformLoadInternalResource("gl.lua");
	int error = luaL_loadbuffer(luaState, (const char*)glScript.buffer, glScript.length, "Cheader");
	error = error | lua_pcall(luaState, 0,0,0);
	if(error)
		LOGE("LUA GL ERROR %s", lua_tostring(luaState, -1));


	Resource coreScript = platformLoadInternalResource("core.lua");
	error = luaL_loadbuffer(luaState, (const char*)coreScript.buffer, coreScript.length, "Cheader");
	error = error | lua_pcall(luaState, 0,0,0);

	if(error)
		LOGE("LUA CORE ERROR %s", lua_tostring(luaState, -1));

	delete glScript.buffer;
	delete coreScript.buffer;

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
		LOGE("LUA SCRIPT ERROR %s in file %s", lua_tostring(L, -1), pathInFiles.c_str());

	delete script.buffer;
}

void loadLuaScripts(lua_State* L, const char* scriptsDirectory)
{
	LOGI("Entered initializeLuaScript");
	std::string scriptsDir(scriptsDirectory);
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir (scriptsDir.c_str())) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if(path::hasExtension(ent->d_name, ".lua")) {
				std::string scriptPath = path::buildPath(scriptsDir, ent->d_name);
				loadLuaScript(L, scriptPath);
				LOGI("Loading script %s", scriptPath.c_str());
			}
		}
		closedir(dir);
	} else {
		LOGE("Couldn't open directory %s", scriptsDir.c_str());
		platformExit();
	}
	LOGI("About to exit initializeLuaScript");
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
		LOGW("LUA Execution Error %s", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static bool callEmptyLuaFunctionBool(lua_State* L, const char* buffer)
{
	lua_getglobal(L, buffer);
	int error = error | lua_pcall(L, 0, 1, 0);
	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}
	/*retrieve result */
	if (!lua_isboolean(L, -1)) {
	 	LOGW("function `f' must return a boolean");
	 	return false;
	}
	auto result = lua_toboolean(L, -1);
	lua_pop(L, 1);  /* pop returned value */

	return result;
}

static void callInt2LuaFunction(lua_State* L, const char* methodName, int x, int y)
{
	char buffer[128];
	sprintf(buffer, "%s(%d,%d)", methodName, x, y);

	int error = luaL_loadbuffer(L, buffer, strlen(buffer), "int int");
	error = error | lua_pcall(L, 0, 0, 0);

	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static void callInt3LuaFunction(lua_State* L, const char* methodName, int x, int y, int z)
{
	char buffer[128];
	sprintf(buffer, "%s(%d,%d,%d)", methodName, x, y, z);

	int error = luaL_loadbuffer(L, buffer, strlen(buffer), "int int int");
	error = error | lua_pcall(L, 0, 0, 0);

	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static void callFloat2LuaFunction(lua_State* L, const char* methodName, float x, float y)
{
	char buffer[128];
	sprintf(buffer, "%s(%f,%f)", methodName, x, y);

	int error = luaL_loadbuffer(L, buffer, strlen(buffer), "float float");
	error = error | lua_pcall(L, 0, 0, 0);

	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static void callFloat4LuaFunction(lua_State* L, const char* methodName, float x1, float y1, float x2, float y2)
{
	char buffer[256];
	sprintf(buffer, "%s(%f,%f,%f,%f)", methodName, x1, y1, x2, y2);

	int error = luaL_loadbuffer(L, buffer, strlen(buffer), "float4");
	error = error | lua_pcall(L, 0, 0, 0);

	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

void luaStopCall(lua_State* L)
{
	callEmptyLuaFunction(L, "stop()");
}

void luaStepCall(lua_State* L)
{
	callEmptyLuaFunction(L, "step()");
}

void luaStartCall(lua_State* L)
{
	callEmptyLuaFunction(L, "start()");
}

void luaRestartCall(lua_State* L)
{
	callEmptyLuaFunction(L, "restart()");
}

void luaHandleMessageCall(lua_State* L, Buffer* buffer, uint32_t id)
{
	lua_getglobal(L, "handleMessage");
	lua_pushlightuserdata(L, buffer);
	lua_pushnumber(L, id);
	auto err = lua_pcall(L, 2, 0, 0);
	if(err) {
		LOGW("LUA Execution Error %s", lua_tostring(L, -1));
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
