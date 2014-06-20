#include "lua_core.h"
#include "lua_core_private.h"
#include <cstring>
#include "game.h"

#include <vector>
#include <lua.hpp>
#include "path.h"

#include "asset_helper.h"

const char* bufferReadLuaString(Buffer* buffer)
{
	char* str;

	bufferReadUTF8(buffer, &str);
	LOGE("OHNO WE CALL READSTR");
	LOGE("%s", str);
	return str;
}

lua_State* luaState;
void initializeLuaCore()
{
	luaState = luaL_newstate();
	luaL_openlibs(luaState);

	Asset glScript("gl.lua");
	int error = luaL_loadbuffer(luaState, (const char*)glScript.buffer, glScript.length, "Cheader");
	error = error | lua_pcall(luaState, 0,0,0);
	if(error)
		LOGE("LUA GL ERROR %s", lua_tostring(luaState, -1));


	Asset coreScript("core.lua");
	error = luaL_loadbuffer(luaState, (const char*)coreScript.buffer, coreScript.length, "Cheader");
	error = error | lua_pcall(luaState, 0,0,0);

	if(error)
		LOGE("LUA CORE ERROR %s", lua_tostring(luaState, -1));

}

#include "dirent.h"

void initializeLuaScripts()
{
	LOGI("Entered initializeLuaScript");
	std::string scriptsDir(gGame->resourceDir);
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir (scriptsDir.c_str())) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if(path::hasExtension(ent->d_name, ".lua")) {
				std::string scriptPath = path::buildPath(scriptsDir, ent->d_name);
				loadLuaScript(scriptPath);
				LOGI("Loading script %s", scriptPath.c_str());
			}
		}
		closedir(dir);
	} else {
		LOGE("Couldn't open directory %s", scriptsDir.c_str());
		exit(-1);
	}
	LOGI("About to exit initializeLuaScript");
}

void loadLuaScript(const std::string& pathInFiles)
{
	ExternalAsset script(pathInFiles);

	int error = luaL_loadbuffer(luaState, (const char*)script.buffer, script.length, pathInFiles.c_str());
	error |= lua_pcall(luaState, 0, 0, 0);

	if(error)
		LOGE("LUA SCRIPT ERROR %s in file %s", lua_tostring(luaState, -1), pathInFiles.c_str());
}

void runLuaGarbageCollector(int milisecs)
{
	uint64_t nsecs = milisecs * 1000000L;

	auto now = timeNowMonoliticNsecs();
	while(timeNowMonoliticNsecs() - now < nsecs)
		if(lua_gc(luaState, LUA_GCSTEP, 0)) break;
}

void callEmptyLuaFunction(const char* buffer)
{
	int error = luaL_loadbuffer(luaState, buffer, strlen(buffer), "empty");
	error = error | lua_pcall(luaState, 0, 0, 0);
	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(luaState, -1));
		lua_pop(luaState, 1);
	}
}

bool callEmptyLuaFunctionBool(const char* buffer)
{
	lua_getglobal(luaState, buffer);
	int error = error | lua_pcall(luaState, 0, 1, 0);
	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(luaState, -1));
		lua_pop(luaState, 1);
		return false;
	}
	/*retrieve result */
	if (!lua_isboolean(luaState, -1)) {
	 	LOGW("function `f' must return a boolean");
	 	return false;
	}
	auto result = lua_toboolean(luaState, -1);
	lua_pop(luaState, 1);  /* pop returned value */

	return result;
}

void callInt2LuaFunction(const char* methodName, int x, int y)
{
	char buffer[128];
	sprintf(buffer, "%s(%d,%d)", methodName, x, y);

	int error = luaL_loadbuffer(luaState, buffer, strlen(buffer), "int int");
	error = error | lua_pcall(luaState, 0, 0, 0);

	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(luaState, -1));
		lua_pop(luaState, 1);
	}
}

void callInt3LuaFunction(const char* methodName, int x, int y, int z)
{
	char buffer[128];
	sprintf(buffer, "%s(%d,%d,%d)", methodName, x, y, z);

	int error = luaL_loadbuffer(luaState, buffer, strlen(buffer), "int int int");
	error = error | lua_pcall(luaState, 0, 0, 0);

	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(luaState, -1));
		lua_pop(luaState, 1);
	}
}

void callFloat2LuaFunction(const char* methodName, float x, float y)
{
	char buffer[128];
	sprintf(buffer, "%s(%f,%f)", methodName, x, y);

	int error = luaL_loadbuffer(luaState, buffer, strlen(buffer), "float float");
	error = error | lua_pcall(luaState, 0, 0, 0);

	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(luaState, -1));
		lua_pop(luaState, 1);
	}
}

void callFloat4LuaFunction(const char* methodName, float x1, float y1, float x2, float y2)
{
	char buffer[256];
	sprintf(buffer, "%s(%f,%f,%f,%f)", methodName, x1, y1, x2, y2);

	int error = luaL_loadbuffer(luaState, buffer, strlen(buffer), "float4");
	error = error | lua_pcall(luaState, 0, 0, 0);

	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(luaState, -1));
		lua_pop(luaState, 1);
	}
}


void resourcesLoadedLuaCall()
{
	callEmptyLuaFunction("resourcesLoaded()");
}

void termLuaCall()
{
	callEmptyLuaFunction("term()");
}

void updateLuaCall()
{
	callEmptyLuaFunction("step()");
}


void callLuaHandleMessage(uint32_t id, uint32_t length)
{
	char buffer[128];
	sprintf(buffer, "handleMessage(%d,%d)", id, length);

	int error = luaL_loadbuffer(luaState, buffer, strlen(buffer), "HandlingMessage");
	error = error | lua_pcall(luaState, 0, 0, 0);

	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(luaState, -1));
		lua_pop(luaState, 1);
	}
}

bool callLuaMenu()
{
	return callEmptyLuaFunctionBool("onMenuButton");
}

bool callLuaBack()
{
	return callEmptyLuaFunctionBool("onBackButton");
}

void luaLog(const char* toLog)
{
	LOGI("%s" , toLog);
}

void luaOnTap(int x, int y)
{
	if(gGame->screen->orientation == ORIENTATION_PORTRAIT)
        callInt2LuaFunction("onTap", y, gGame->screen->width-x);
	else
        callInt2LuaFunction("onTap", x, y);
}

void luaOnTouch(int x, int y, int pointerIndex)
{
	if(gGame->screen->orientation == ORIENTATION_PORTRAIT)
        callInt3LuaFunction("onTouch", y,
                gGame->screen->width-x, pointerIndex);
	else
        callInt3LuaFunction("onTouch", x, y, pointerIndex);
}

void luaOnDrag(float x, float y)
{
	if(gGame->screen->orientation == ORIENTATION_PORTRAIT)
        callFloat2LuaFunction("onDrag", y,
                gGame->screen->width-x);
	else
        callFloat2LuaFunction("onDrag", x, y);
}

void luaOnDragBegin(float x, float y)
{
	if(gGame->screen->orientation == ORIENTATION_PORTRAIT)
        callFloat2LuaFunction("onDragBegin", y, gGame->screen->width-x);
	else
        callFloat2LuaFunction("onDragBegin", x, y);
}

void luaOnDragEnd(float x, float y)
{
	if(gGame->screen->orientation == ORIENTATION_PORTRAIT)
        callFloat2LuaFunction("onDragEnd", y, gGame->screen->width-x);
	else
        callFloat2LuaFunction("onDragEnd", x, y);
}

void luaOnPinchBegin(float x1, float y1, float x2, float y2)
{
	if(gGame->screen->orientation == ORIENTATION_PORTRAIT)
		callFloat4LuaFunction("onPinchBegin",
			y1, gGame->screen->width-x1,
			y2, gGame->screen->width-x2);
	else
		callFloat4LuaFunction("onPinchBegin", x1, y1, x2, y2);
}

void luaOnPinch(float x1, float y1, float x2, float y2)
{
	if(gGame->screen->orientation == ORIENTATION_PORTRAIT)
		callFloat4LuaFunction("onPinch",
			y1, gGame->screen->width-x1,
			y2, gGame->screen->width-x2);
	else
		callFloat4LuaFunction("onPinch", x1, y1, x2, y2);
}


