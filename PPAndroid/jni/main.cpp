/*
 * main.cpp
 *
 *  Created on: Feb 15, 2014
 *      Author: Lukas_2
 */

#include "main.h"

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


struct AppState
{
	bool isStarted,
		 isResumed,
		 isFocused,
		 hasSurface,
		 wasStopped;

	bool fullyActive()
	{
		return isFocused && isResumed && isStarted && hasSurface;
	}


};

AppState gAppState;

namespace lifecycle
{
	void create()
	{
		context = ndk_helper::GLContext::GetInstance();

		gAppState.isStarted 	= false;
		gAppState.isResumed 	= false;
		gAppState.isFocused 	= false;
		gAppState.hasSurface 	= false;
		gAppState.wasStopped 	= false;
	}

	void start()
	{
		if(gAppState.wasStopped)
			restart();
		else
			freshStart();
	}

	void restart()
	{
		LOGI("App was restarted!");
		gAppState.isStarted = true;
	}

	void freshStart()
	{
		LOGI("App was started!");
		gAppState.isStarted = true;
	}

	void resume()
	{
		LOGI("App was resumed!");
		gAppState.isResumed = true;
	}

	void pause()
	{
		LOGI("App was paused!");
		gAppState.isResumed = false;
		gAppState.isFocused = false;

	}

	void stop()
	{
		LOGI("App was stopped!");

		gAppState.wasStopped = true;
		gAppState.isStarted  = false;
	}

	void destroy()
	{
		LOGI("App was destroyed!");
	}

	//Focus Events
	void gainedFocus()
	{
		LOGI("App gained focus");
		gAppState.isFocused = true;
	}

	void lostFocus()
	{
		LOGI("App lost focus");
		gAppState.isFocused = false;
	}

	//EGL calls
	void surfaceCreated()
	{
		LOGI("Surface created graphics resources can be safely loaded.");
		gAppState.hasSurface = true;

		context ->Init(gApp->window);

		gameInitialize(gApp);
		gameStart();
		isInitialized = true;

	}

	void surfaceDestroyed()
	{
		LOGI("Surface Destroyed");
		gAppState.hasSurface = false;

		context->Invalidate();
	}

	void surfaceChanged()
	{
		LOGI("Surface Changed");
	}
}

void handle_cmd(android_app* app, int32_t cmd)
{
	switch(cmd)
	{
		//Basic Lifecycle events
		case APP_CMD_START: 	lifecycle::start();		break;
		case APP_CMD_RESUME: 	lifecycle::resume(); 	break;
		case APP_CMD_PAUSE:		lifecycle::pause(); 	break;
		case APP_CMD_STOP:		lifecycle::stop();		break;
		case APP_CMD_DESTROY:	lifecycle::destroy(); 	break;

		//Focus events
		case APP_CMD_GAINED_FOCUS: 	lifecycle::gainedFocus(); 	break;
		case APP_CMD_LOST_FOCUS: 	lifecycle::lostFocus(); 	break;

		//Window events
		case APP_CMD_INIT_WINDOW: 	 lifecycle::surfaceCreated();   break;
		case APP_CMD_TERM_WINDOW: 	 lifecycle::surfaceDestroyed();	break;
		case APP_CMD_WINDOW_RESIZED: lifecycle::surfaceChanged();	break;
		case APP_CMD_CONFIG_CHANGED: lifecycle::surfaceChanged();	break;
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
	state->onAppCmd 		= &handle_cmd;
	state->onInputEvent 	= &handle_input;
	gApp = state;

	lifecycle::create();

	while(1)
	{
		int ident, fdesc, events;
		android_poll_source* source;

		while((ident = ALooper_pollOnce(0, &fdesc, &events, (void**)&source)) >= 0)
		{
		   if(source)
			   source->process(state, source);

		   if(state->destroyRequested)
			   return;
		}

		if(gAppState.fullyActive()) {
			gameStep(context);
		}
	}

	LOGI("Native Activity Was Fully Destroyed!");
}
