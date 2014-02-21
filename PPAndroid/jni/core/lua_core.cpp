

#define GLM_FORCE_RADIANS

#include "lua_core.h"
#include "lua_core_private.h"
#include <cstring>

#include "Renderer.h"
#include <vector>
#include "font_loading.h"
#include "image_loader.h"
#include "types.h"

#include <NDKHelper.h>
#include <android_native_app_glue.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <lua.hpp>

Renderer* gRenderer;
android_app* gApp;

std::vector<Font> fonts;
std::vector<Frame> frames;

struct Asset
{
	size_t length;
	uint8_t* buffer;
	AAsset* asset;

	Asset(const char* fileName)
	{
		auto mgr   = gApp->activity->assetManager;
		asset = AAssetManager_open(mgr, fileName, AASSET_MODE_RANDOM);
		length = AAsset_getLength(asset);
		buffer = (uint8_t*)AAsset_getBuffer(asset);

	}

	~Asset()
	{
		AAsset_close(asset);
	}
};


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

uint32_t loadFrame(const char* name)
{
	auto texture = loadTexture(name);
	auto frame   = Frame(texture, glm::vec4(0,1,1,-1));

	LOGI("LOADING FRAMES LIKE A MINOR BAWNS");

	frames.push_back(frame);
	return frames.size() - 1;
}

void unloadFrame(uint32_t frameHandle)
{
}

void addFrame(uint32_t frameHandle, vec2 pos, vec2 dim, uint32_t color)
{
	LOGI("pos: %f,%f dim: %f,%f", pos.x, pos.y, dim.x, dim.y);

	auto frame = frames[frameHandle];
	gRenderer->addFrame(frame, glm::vec2(pos.x, pos.y), glm::vec2(dim.x, dim.y), Color(color));
}

void addFrame2(uint32_t frameHandle, vec2 pos, vec2 dim, uint32_t color,
			   vec2 origin, float rotation, int mirror)
{
	LOGI("pos: %f,%f dim: %f,%f", pos.x, pos.y, dim.x, dim.y);

	auto frame = frames[frameHandle];
	gRenderer->addFrame(frame, glm::vec2(pos.x, pos.y), glm::vec2(dim.x, dim.y), Color(color),
						glm::vec2(origin.x, origin.y), rotation, mirror == 0 ? false : true);
}

void addText(uint32_t fontHandle, const char* str, vec2 pos, uint32_t color)
{
	auto font = fonts[fontHandle];
	gRenderer->addText(font, str, glm::vec2(pos.x, pos.y), Color(color));
}

lua_State* luaState;

void initializeLuaCore(android_app* app)
{
	gApp = app;
	gRenderer = new Renderer(1024);

	luaState = luaL_newstate();
	luaL_openlibs(luaState);


	Asset coreScript("Core.lua");
	int error = luaL_loadbuffer(luaState, (const char*)coreScript.buffer, coreScript.length, "Cheader");
	error = error | lua_pcall(luaState, 0,0,0);

	if(error)
	{
		LOGI("ERROR LUA 1");
	}


	Asset script("Script.lua");

	error |= luaL_loadbuffer(luaState, (const char*)script.buffer, script.length, "CData");
	error |= lua_pcall(luaState, 0, 0, 0);

	if(error)
	{
		LOGI("ERROR LUA 2");
	}
}

void callEmptyLuaFunction(const char* buffer)
{
	int error = luaL_loadbuffer(luaState, buffer, strlen(buffer), "weeee");
	error = error | lua_pcall(luaState, 0, 0, 0);

	if(error) {
		LOGI("LUA STATE ERROR!");
	}
}

void initLuaCall()
{
	LOGI("Calling init like a baws :)");
	callEmptyLuaFunction("init()");
}

void termLuaCall()
{
	callEmptyLuaFunction("term()");
}

void updateLuaCall()
{
	callEmptyLuaFunction("update()");
}

void renderLuaCall(ndk_helper::GLContext* context)
{
	glClearColor(0.5,0.5,0.5,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 matrix = glm::ortho(0.0f, (float)context->GetScreenWidth(),
		  0.0f, (float)context->GetScreenHeight());
	gRenderer->setTransform(matrix);

	callEmptyLuaFunction("render()");

	gRenderer->draw();
	context->Swap();
}
