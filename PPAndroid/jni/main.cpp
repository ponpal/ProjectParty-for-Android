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

    auto tmp = env->FindClass("projectparty/ppandroid/services/ControllerService");
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
ASensorManager* gSensorManager;
ASensor* gAccelerometerSensor;
ASensorEventQueue* gSensorEventQueue;

void initSensors()
{
	gSensorManager = ASensorManager_getInstance();
	gAccelerometerSensor = (ASensor*)ASensorManager_getDefaultSensor(gSensorManager, ASENSOR_TYPE_ACCELEROMETER);
	gSensorEventQueue = ASensorManager_createEventQueue(gSensorManager, gApp->looper, LOOPER_ID_USER, NULL, NULL);
}

void resumeSensors()
{

	ASensorEventQueue_enableSensor(gSensorEventQueue, gAccelerometerSensor);
	ASensorEventQueue_setEventRate(gSensorEventQueue, gAccelerometerSensor, (1000L / 60) * 1000);
}

void pauseSensors()
{
	ASensorEventQueue_disableSensor(gSensorEventQueue, gAccelerometerSensor);
}

void processSensors(int32_t id)
{
	if(id == LOOPER_ID_USER)
	{
		ASensorEvent event;
		while(ASensorEventQueue_getEvents(gSensorEventQueue, &event, 1))
		{
			switch(event.type)
			{
				case ASENSOR_TYPE_ACCELEROMETER:
					vec3 v;
					v.x = event.acceleration.x;
					v.y = event.acceleration.y;
					v.z = event.acceleration.z;

					gGame->sensor->acceleration = v;
				break;
			}
		}
	}
}

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

		gameRestart();
	}

	void freshStart()
	{
		LOGI("App was started!");
		gAppState.isStarted = true;

		gameStart();
	}

	void resume()
	{
		LOGI("App was resumed!");
		gAppState.isResumed = true;

		gameResume();
	}

	void pause()
	{
		LOGI("App was paused!");
		gAppState.isResumed = false;
		gAppState.isFocused = false;

		gamePause();
	}

	void stop()
	{
		LOGI("App was stopped!");

		gAppState.wasStopped = true;
		gAppState.isStarted  = false;

		gameStop();
	}

	void destroy()
	{
		LOGI("App was destroyed!");
		networkShutdown(gGame->network);
	}

	//Focus Events
	void gainedFocus()
	{
		LOGI("App gained focus");
		gAppState.isFocused = true;
		resumeSensors();
	}

	void lostFocus()
	{
		LOGI("App lost focus");
		gAppState.isFocused = false;
		pauseSensors();
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

		gameSurfaceCreated();

	}

	void surfaceDestroyed()
	{
		LOGI("Surface Destroyed");
		gAppState.hasSurface = false;
		gameStop();

		context->Invalidate();

		gameSurfaceDestroyed();
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

	initSensors();
	lifecycle::create();

	while(1)
	{
		int ident, fdesc, events;
		android_poll_source* source;

		while((ident = ALooper_pollOnce(0, &fdesc, &events, (void**)&source)) >= 0)
		{
			if(source)
			   source->process(state, source);

		   processSensors(ident);

		   if(state->destroyRequested)
			   return;
		}

		if(gAppState.fullyActive()) {
			gameStep(context);
		}
	}

	LOGI("Native Activity Was Fully Destroyed!");
}
