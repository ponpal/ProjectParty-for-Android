

#define GLM_FORCE_RADIANS

#include "lua_core.h"
#include "lua_core_private.h"
#include <cstring>
#include "game.h"
#include <cmath>

#include "new_renderer.h"
#include <vector>
#include "font_loading.h"
#include "image_loader.h"
#include "types.h"
#include "font.h"
#include "time_helpers.h"

#include "JNIHelper.h"
#include <android_native_app_glue.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <lua.hpp>
#include "android_helper.h"
#include "asset_helper.h"
#include "content.h"
#include "path.h"

uint32_t loadFont(const char* fontName)
{
	std::string path(gGame->name);
	path += "/";
	path += fontName;
	return gGame->content->loadFont(path);
}

void unloadFont(uint32_t fontHandle)
{
	gGame->content->unloadFont(fontHandle);
}

vec2f measureString(uint32_t fontHandle, const char* str)
{
	auto f = gGame->content->getFont(fontHandle);
	auto vec = font::measure(f, str);

	vec2f v;
	v.x = vec.x;
	v.y = vec.y;
	return v;
}

const char* bufferReadLuaString(Buffer* buffer)
{
	char* str;

	bufferReadUTF8(buffer, &str);
	LOGE("OHNO WE CALL READSTR");
	LOGE("%s", str);
	return str;
}

const char* testStr()
{
	return "a";
}

uint32_t loadFrame(const char* name)
{
	std::string path(gGame->name);
	path += "/";
	path += name;
	return gGame->content->loadFrame(path);
}

void unloadFrame(uint32_t frameHandle)
{
	gGame->content->unloadFrame(frameHandle);
}

void addFrame(uint32_t frameHandle, vec2f pos, vec2f dim, uint32_t color)
{
	auto frame = gGame->content->getFrame(frameHandle);
	rendererAddFrame(gGame->renderer, &frame, pos, dim, color);
}

void addFrame2(uint32_t frameHandle, vec2f pos, vec2f dim, uint32_t color,
		vec2f origin, float rotation, int mirror)
{
	auto frame = gGame->content->getFrame(frameHandle);
	rendererAddFrame2(gGame->renderer, &frame, pos, dim, color, origin, rotation, mirror);
}

void addText(uint32_t fontHandle, const char* str, vec2f pos, uint32_t color)
{
	auto font = gGame->content->getFont(fontHandle);
	rendererAddText(gGame->renderer, &font, str, pos, color);
}

lua_State* luaState;
void initializeLuaCore()
{
	luaState = luaL_newstate();
	luaL_openlibs(luaState);

	Asset coreScript("Core.lua");
	int error = luaL_loadbuffer(luaState, (const char*)coreScript.buffer, coreScript.length, "Cheader");
	error = error | lua_pcall(luaState, 0,0,0);

	if(error)
		LOGE("LUA CORE ERROR %s", lua_tostring(luaState, -1));
}

#include "dirent.h"

void initializeLuaScripts(const std::string& dirName)
{
	LOGI("Entered initializeLuaScript");
	std::string scriptsDir;
	scriptsDir += gApp->activity->externalDataPath;
	scriptsDir += "/" + dirName + "/scripts/";
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir (scriptsDir.c_str())) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if(path::hasExtension(ent->d_name, ".lua")) {
				std::string scriptPath = "/" + dirName + "/scripts/";
				scriptPath += ent->d_name;
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

	int error = luaL_loadbuffer(luaState, (const char*)script.buffer, script.length, "CData");
	error |= lua_pcall(luaState, 0, 0, 0);

	if(error)
		LOGE("LUA SCRIPT ERROR %s", lua_tostring(luaState, -1));
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
	LOGI("callEmpty entered" );
	int error = luaL_loadbuffer(luaState, buffer, strlen(buffer), "empty");
	LOGI("callEmpty error1 %d", error );
	error = error | lua_pcall(luaState, 0, 0, 0);
	LOGI("CallEmpty error2 %d", error);
	if(error) {
		LOGW("LUA Execution Error %s", lua_tostring(luaState, -1));
		lua_pop(luaState, 1);
	}
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


void initLuaCall()
{
	callEmptyLuaFunction("init()");
}

void termLuaCall()
{
	callEmptyLuaFunction("term()");
}

void updateLuaCall()
{
	callEmptyLuaFunction("coreUpdate() update()");
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

void renderLuaCall()
{
	glClearColor(0.5,0.5,0.5,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0,0, gGame->screen->width, gGame->screen->height);

	glm::mat4 matrix = glm::ortho(0.0f, (float)gGame->screen->width,
		  0.0f, (float)gGame->screen->height);
	//matrix = translate(matrix, glm::vec3(800,0,0));
	//matrix = rotate(matrix, (float)M_PI_2, glm::vec3(0,0,1));

	rendererSetTransform(gGame->renderer, &matrix);
	callEmptyLuaFunction("render()");
	rendererDraw(gGame->renderer);
}

void luaLog(const char* toLog)
{
	LOGI("%s" , toLog);
}

void luaOnTap(int x, int y)
{
	callInt2LuaFunction("onTap", x, y);
}

void luaOnTouch(int x, int y, int pointerIndex)
{
	callInt3LuaFunction("onTouch", x, y, pointerIndex);
}

void luaOnDrag(float x, float y)
{
	callFloat2LuaFunction("onDrag", x, y);
}

void luaOnDragBegin(float x, float y)
{
	callFloat2LuaFunction("onDragBegin", x, y);
}

void luaOnDragEnd(float x, float y)
{
	callFloat2LuaFunction("onDragEnd", x, y);
}

void luaOnPinchBegin(float x1, float y1, float x2, float y2)
{
	callFloat4LuaFunction("onPinchBegin", x1, y1, x2, y2);
}

void luaOnPinch(float x1, float y1, float x2, float y2)
{
	callFloat4LuaFunction("onPinch", x1, y1, x2, y2);
}


