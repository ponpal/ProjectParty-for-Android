/*
 * main.cpp
 *
 *  Created on: Feb 15, 2014
 *      Author: Lukas_2
 */

#define GLM_FORCE_RADIANS

#include <android/log.h>
#include <android_native_app_glue.h>
#include <NDKHelper.h>
#include <vector>
#include "core/lua_core_private.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#include "core/Renderer.h"
#include "core/types.h"
#include "core/image_loader.h"
#include "core/font_loading.h"
#include "core/game.h"
#include "core/android_helper.h"

#define HELPER_CLASS_NAME "projectparty/ppandroid/NDKHelper"

static int32_t handle_input(android_app* app, AInputEvent* event)
{
	auto type = AInputEvent_getType(event);
	if(type == AINPUT_EVENT_TYPE_MOTION)
	{
		size_t pointerCount = AMotionEvent_getPointerCount(event);
		for(size_t i = 0; i < pointerCount; i++)
		{
			LOGI("Received motion event from pointer %zu: (%.2f %.2f)",
				 i, AMotionEvent_getX(event, i), AMotionEvent_getY(event, i));
		}
		return 1;
	}
	else if(type == AINPUT_EVENT_TYPE_KEY)
	{
		LOGI("Received key event: %d", AKeyEvent_getKeyCode(event));
		return 0;
	}

	return 0;
}

ndk_helper::GLContext* context;

bool isInitialized;
bool noRender;


extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    LOGI("JNI_OnLoad was called!");

    auto tmp = env->FindClass("projectparty/ppandroid/ControllerService");
    LOGI("Loaded class! %d ", (size_t)tmp);
    auto clazz = (jclass)env->NewGlobalRef(tmp);

    networkServiceClass(clazz);

    return JNI_VERSION_1_6;
}

void onCreate()
{
	LOGI("App was created!");
}

void onStart()
{
	LOGI("App was started!");
}

void onResume()
{
	LOGI("App was resumed!");
}

void onPause()
{
	LOGI("App was paused!");
}

void onStop()
{
	LOGI("App was stopped!");
}

void handle_cmd(android_app* app, int32_t cmd)
{
	switch(cmd)
	{
		case APP_CMD_SAVE_STATE:
			LOGI("Command save state.");
			break;
		case APP_CMD_INIT_WINDOW:
			LOGI("Window initialized.");
			context ->Init(app->window);
			gameInitialize(app);
			gameStart();
			isInitialized = true;
	        break;
		case APP_CMD_TERM_WINDOW:
			LOGI("Window terminated");
			context->Invalidate();
			break;
		case APP_CMD_LOST_FOCUS:
			LOGI("Gained lost focus");
			break;
		case APP_CMD_GAINED_FOCUS:
			LOGI("Gained focus");
			break;
		case APP_CMD_INPUT_CHANGED:
			LOGI("Input changed");
			break;
		case APP_CMD_WINDOW_RESIZED:
			LOGI("Window resized");
			break;
		case APP_CMD_WINDOW_REDRAW_NEEDED:
			LOGI("App redraw needed!");
			break;
		case APP_CMD_CONTENT_RECT_CHANGED:
			LOGI("App Rect Changed!");
			break;
		case APP_CMD_CONFIG_CHANGED:
			LOGI("App Config Changed!");
			break;
		case APP_CMD_START:
			LOGI("App started!");
			networkInitialize(app);
			break;
		case APP_CMD_RESUME:
			LOGI("App Resumed!");
			break;
		case APP_CMD_STOP:
			LOGI("App was stopped state.");
			break;
		case APP_CMD_DESTROY:
			LOGI("App is being destroyed.");
			context->Invalidate();
			break;
	}
}

void render()
{
	if(!isInitialized) return;
	updateLuaCall();
	renderLuaCall(context);
}

void android_main(android_app* state)
{
	app_dummy();

	isInitialized = false;

	state->onAppCmd 		= &handle_cmd;
	state->onInputEvent 	= &handle_input;

	ndk_helper::JNIHelper::Init(state->activity, HELPER_CLASS_NAME);
	context = ndk_helper::GLContext::GetInstance();

	gApp = state;


	int frame = 0;
	while(1)
	{
		int ident, fdesc, events;
		android_poll_source* source;

		while((ident = ALooper_pollOnce(0, &fdesc, &events, (void**)&source)) >= 0)
		{
		   //Do something wonderful with the event.
		   if(source)
			   source->process(state, source);

		   if(state->destroyRequested)
			   return;
		}
		if(isInitialized) {
			gameStep(context);
			frame++;
		}
	}
}
