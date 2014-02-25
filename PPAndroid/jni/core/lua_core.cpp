

#define GLM_FORCE_RADIANS

#include "lua_core.h"
#include "lua_core_private.h"
#include <cstring>
#include "game.h"

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


std::vector<Font> fonts;
std::vector<Frame> frames;

uint32_t loadFont(const char* frameName, const char* fontName)
{
	auto frame = loadFrame(frameName);
	auto mgr   = gApp->activity->assetManager;

	Asset asset(fontName);
	auto font = constructFont(asset.buffer, asset.length, frames[frame]);
	fonts.push_back(font);

	return fonts.size() - 1;
}

void unloadFont(uint32_t fontHandle)
{
}

vec2f measureString(uint32_t fontHandle, const char* str)
{
	auto f = fonts[fontHandle];
	auto vec = font::measure(f, str);

	vec2f v;
	v.x = vec.x;
	v.y = vec.y;
	return v;
}

uint32_t loadFrame(const char* name)
{
	auto texture = loadTexture(name);
	auto frame   = Frame(texture, glm::vec4(0,1,1,-1));
	frames.push_back(frame);
	return frames.size() - 1;
}

void unloadFrame(uint32_t frameHandle)
{
}

void addFrame(uint32_t frameHandle, vec2f pos, vec2f dim, uint32_t color)
{
	rendererAddFrame(gGame->renderer, &frames[frameHandle], pos, dim, color);
}

void addFrame2(uint32_t frameHandle, vec2f pos, vec2f dim, uint32_t color,
		vec2f origin, float rotation, int mirror)
{
	rendererAddFrame2(gGame->renderer, &frames[frameHandle], pos, dim, color, origin, rotation, mirror);
}

void addText(uint32_t fontHandle, const char* str, vec2f pos, uint32_t color)
{
	rendererAddText(gGame->renderer, &fonts[fontHandle], str, pos, color);
}

lua_State* luaState;
void initializeLuaCore()
{
	luaState = luaL_newstate();
	luaL_openlibs(luaState);

	const char* fileName = "app_config.xml";

	const char* content = "asdfjkl;asdfjkl;\0";
	{
        ExternalAsset assetWrite = ExternalAsset(std::string(fileName), content, 17);
	}
	ExternalAsset assetRead = ExternalAsset(content);


	Asset coreScript("Core.lua");
	int error = luaL_loadbuffer(luaState, (const char*)coreScript.buffer, coreScript.length, "Cheader");
	error = error | lua_pcall(luaState, 0,0,0);

	if(error)
		LOGI("LUA CORE ERROR %s", lua_tostring(luaState, -1));
}

void initializeLuaScripts()
{
	Asset script("Script.lua");
	int error = luaL_loadbuffer(luaState, (const char*)script.buffer, script.length, "CData");
	error |= lua_pcall(luaState, 0, 0, 0);

	if(error)
		LOGI("LUA SCRIPT ERROR %s", lua_tostring(luaState, -1));
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
	int error = luaL_loadbuffer(luaState, buffer, strlen(buffer), "weeee");
	error = error | lua_pcall(luaState, 0, 0, 0);

	if(error) {
		LOGI("LUA Execution Error %s", lua_tostring(luaState, -1));
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

void renderLuaCall()
{
	glClearColor(0.5,0.5,0.5,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0,0, gGame->screen->width, gGame->screen->height);

	glm::mat4 matrix = glm::ortho(0.0f, (float)gGame->screen->width,
		  0.0f, (float)gGame->screen->height);
	rendererSetTransform(gGame->renderer, &matrix);
	callEmptyLuaFunction("render()");
	rendererDraw(gGame->renderer);
}

void luaLog(const char* toLog)
{
	LOGI("%s" , toLog);
}
