/*
 * main.cpp
 *
 *  Created on: Feb 15, 2014
 *      Author: Lukas_2
 */

#include "main.h"
#include "sys/stat.h"
#include "errno.h"
#include "core/remote_debug.h"
#include "unistd.h"

ndk_helper::GLContext* context;

int getUnicodeChar(struct android_app* app, int eventType, int keyCode, int metaState)
{
	JavaVM* javaVM = app->activity->vm;
	JNIEnv* jniEnv = app->activity->env;

	JavaVMAttachArgs attachArgs;
	attachArgs.version = JNI_VERSION_1_6;
	attachArgs.name = "NativeThread";
	attachArgs.group = NULL;

	jint result = javaVM->AttachCurrentThread(&jniEnv, &attachArgs);
	if(result == JNI_ERR)
	{
		return 0;
	}

	jclass class_key_event = jniEnv->FindClass("android/view/KeyEvent");
	int unicodeKey;

	if(metaState == 0)
	{
		jmethodID method_get_unicode_char = jniEnv->GetMethodID(class_key_event, "getUnicodeChar", "()I");
		jmethodID eventConstructor = jniEnv->GetMethodID(class_key_event, "<init>", "(II)V");
		jobject eventObj = jniEnv->NewObject(class_key_event, eventConstructor, eventType, keyCode);

		unicodeKey = jniEnv->CallIntMethod(eventObj, method_get_unicode_char);
	}
	else
	{
		jmethodID method_get_unicode_char = jniEnv->GetMethodID(class_key_event, "getUnicodeChar", "(I)I");
		jmethodID eventConstructor = jniEnv->GetMethodID(class_key_event, "<init>", "(II)V");
		jobject eventObj = jniEnv->NewObject(class_key_event, eventConstructor, eventType, keyCode);

		unicodeKey = jniEnv->CallIntMethod(eventObj, method_get_unicode_char, metaState);
	}

	javaVM->DetachCurrentThread();

	return unicodeKey;
}


static int32_t handle_input(android_app* app, AInputEvent* event) {
	if(!gGame)
		return 0;

	auto type = AInputEvent_getType(event);

	if (type == AINPUT_EVENT_TYPE_MOTION) {
	    int32_t action = AMotionEvent_getAction( event );
	    uint32_t flags = action & AMOTION_EVENT_ACTION_MASK;
	    auto event_ = event;

	    int32_t count = AMotionEvent_getPointerCount( event );
	    int32_t index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
	                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
	    uint32_t pointerID;
	    float x, y;
	    switch( flags )
	    {
	    case AMOTION_EVENT_ACTION_DOWN:
            pointerID = AMotionEvent_getPointerId( event, 0);
            x = AMotionEvent_getX(event, 0);
            y = AMotionEvent_getY(event, 0);
            luaOnDownCall(gGame->L, pointerID, x, y);
	        break;
	    case AMOTION_EVENT_ACTION_POINTER_DOWN:
	    {
            pointerID = AMotionEvent_getPointerId( event, index);
            x = AMotionEvent_getX(event, index);
            y = AMotionEvent_getY(event, index);
            luaOnDownCall(gGame->L, pointerID, x, y);
	    }
	        break;
	    case AMOTION_EVENT_ACTION_UP:
            pointerID = AMotionEvent_getPointerId( event, index);
            x = AMotionEvent_getX(event, index);
            y = AMotionEvent_getY(event, index);
            luaOnUpCall(gGame->L, pointerID, x, y);
	        break;
	    case AMOTION_EVENT_ACTION_POINTER_UP:
	    {
            pointerID = AMotionEvent_getPointerId( event, index);
            x = AMotionEvent_getX(event, index);
            y = AMotionEvent_getY(event, index);
            luaOnUpCall(gGame->L, pointerID, x, y);
	    }
	        break;
	    case AMOTION_EVENT_ACTION_MOVE:
	        for(int i = 0; i < count; i++) {
	        	pointerID = AMotionEvent_getPointerId( event, i);
	        	x = AMotionEvent_getX(event, i);
	        	y = AMotionEvent_getY(event, i);
                luaOnMoveCall(gGame->L, pointerID, x, y);
	        }
	        break;
	    case AMOTION_EVENT_ACTION_CANCEL:
	        for(int i = 0; i < count; i++) {
	        	pointerID = AMotionEvent_getPointerId( event, i);
	        	x = AMotionEvent_getX(event, i);
	        	y = AMotionEvent_getY(event, i);
                luaOnCancelCall(gGame->L, pointerID, x, y);
	        }
	        break;
	    }
	} else if (type == AINPUT_EVENT_TYPE_KEY) {
		auto code = AKeyEvent_getKeyCode(event);
		RLOGI("Received key event: %d", code);

		switch(code) {
            case AKEYCODE_MENU:
            	return luaMenuCall(gGame->L);
                break;
            case AKEYCODE_BACK:
            	RLOGI("BACK KEY PRESSED %s", "");
            	return luaBackCall(gGame->L);
                break;
            default:
            	return 0;
		}
	}

	return 0;
}

struct AppState {
	bool isStarted, isResumed, isFocused, hasSurface, wasStopped;

	bool fullyActive() {
		return isFocused && isResumed && isStarted && hasSurface;
	}
};

AppState gAppState;
ASensorManager* gSensorManager;
ASensor* gAccelerometerSensor;
ASensorEventQueue* gSensorEventQueue;

void initSensors() {
	gSensorManager = ASensorManager_getInstance();
	gAccelerometerSensor = (ASensor*) ASensorManager_getDefaultSensor(
			gSensorManager, ASENSOR_TYPE_ACCELEROMETER);
	gSensorEventQueue = ASensorManager_createEventQueue(gSensorManager,
			gApp->looper, LOOPER_ID_USER, NULL, NULL);
}

void resumeSensors() {

	ASensorEventQueue_enableSensor(gSensorEventQueue, gAccelerometerSensor);
	ASensorEventQueue_setEventRate(gSensorEventQueue, gAccelerometerSensor,
			(1000L / 60) * 1000);
}

void pauseSensors() {
	ASensorEventQueue_disableSensor(gSensorEventQueue, gAccelerometerSensor);
}

void processSensors(int32_t id) {
	if (id == LOOPER_ID_USER) {
		ASensorEvent event;
		while (ASensorEventQueue_getEvents(gSensorEventQueue, &event, 1) > 0) {
			switch (event.type) {
			case ASENSOR_TYPE_ACCELEROMETER:
				vec3f v;
				v.x = event.acceleration.x;
				v.y = event.acceleration.y;
				v.z = event.acceleration.z;

                if (gameInitialized())
                    gGame->sensor->acceleration = v;
                break;
			}
		}
	}
}

namespace lifecycle {
void create() {
	context = ndk_helper::GLContext::GetInstance();

	gAppState.isStarted = false;
	gAppState.isResumed = false;
	gAppState.isFocused = false;
	gAppState.hasSurface = false;
	gAppState.wasStopped = false;

}

void start() {
	if (gAppState.wasStopped)
		restart();
	else
		freshStart();
}

void restart() {
	RLOGI("%s", "App was restarted!");
	gAppState.isStarted = true;
}

void freshStart() {
	RLOGI("%s", "App was started!");
	gAppState.isStarted = true;
}

void resume() {
	RLOGI("%s", "App was resumed!");
	gAppState.isResumed = true;
}

void pause() {
	RLOGI("%s", "App was paused!");
	gAppState.isResumed = false;
	gAppState.isFocused = false;
}

void stop() {
	RLOGI("s", "App was stopped!");

	gAppState.wasStopped = true;
	gAppState.isStarted = false;

	gameStop();
}

void destroy() {
	RLOGI("%s", "App was destroyed!");
}

//Focus Events
void gainedFocus() {
	RLOGI("%s", "App gained focus");
	gAppState.isFocused = true;
	resumeSensors();
}

void lostFocus() {
	RLOGI("%s", "App lost focus");
	gAppState.isFocused = false;
	pauseSensors();
}

//EGL calls
void surfaceCreated() {
	RLOGI("%s", "Surface created graphics resources can be safely loaded.");
	gAppState.hasSurface = true;

	context->Init(gApp->window);
	if (gameInitialized()) {
        gGame->screen->width = context->GetScreenWidth();
        gGame->screen->height = context->GetScreenHeight();
	}
}

void surfaceDestroyed() {
	RLOGI("%s", "Surface Destroyed");
	gAppState.hasSurface = false;

	context->Invalidate();
	gameStop();
}

void surfaceChanged() {
	RLOGI("%s", "Surface Changed");
	if (gameInitialized()) {
        gGame->screen->width = context->GetScreenWidth();
        gGame->screen->height = context->GetScreenHeight();
		RLOGI("Updating game screen width and height %d %d", gGame->screen->width,
			  gGame->screen->height);
	}
}
}

void handle_cmd(android_app* app, int32_t cmd) {
	switch (cmd) {

	//Basic Lifecycle events
	case APP_CMD_START:
		lifecycle::start();
		break;
	case APP_CMD_RESUME:
		lifecycle::resume();
		break;
	case APP_CMD_PAUSE:
		lifecycle::pause();
		break;
	case APP_CMD_STOP:
		lifecycle::stop();
		break;
	case APP_CMD_DESTROY:
		lifecycle::destroy();
		break;

		//Focus events
	case APP_CMD_GAINED_FOCUS:
		lifecycle::gainedFocus();
		break;
	case APP_CMD_LOST_FOCUS:
		lifecycle::lostFocus();
		break;

		//Window events
	case APP_CMD_INIT_WINDOW:
		lifecycle::surfaceCreated();
		break;
	case APP_CMD_TERM_WINDOW:
		lifecycle::surfaceDestroyed();
		break;
	case APP_CMD_WINDOW_RESIZED:
		lifecycle::surfaceChanged();
		break;
	case APP_CMD_CONFIG_CHANGED:
		lifecycle::surfaceChanged();
		break;
	}
}

void initializeFileSystem() {
	//On some phones the directory does not exist so we create it :)
	auto externalsDir = gApp->activity->externalDataPath;
	std::string d(externalsDir);
	RLOGI("EXTERNALSDIR: %s", d.c_str());
	d.erase(d.size() - 6, 6);

	int err = mkdir(d.c_str(), 0770);
	if (err != 0 && errno != 17)
		RLOGI("Error is: %d %s", errno, strerror(errno));

	err = mkdir(externalsDir, 0770);
	if (err != 0 && errno != 17)
		RLOGI("Error is: %d %s", errno, strerror(errno));
}


void sighandler(int signum)
{
	switch(signum)
	{
		case SIGSEGV:
			RLOGE("Crash due to %d SIGSEGV! (invalid memory access)", signum);
			break;
		case SIGFPE:
			RLOGE("Crash due to %d SIGFPE! (divide by zero)", signum);
			break;
		default:
			RLOGE("Crash due to %d signal!", signum);
			break;
	}



	gameStop();
	RLOGE("%s", "Exiting application");
	sleep(1);
	exit(-1);
}


static void process_input( struct android_app* app, struct android_poll_source* source) {
    AInputEvent* event = NULL;
    if (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
        int type = AInputEvent_getType(event);
        //LOGV("New input event: type=%d\n", AInputEvent_getType(event));

        bool skip_predispatch
              =  AInputEvent_getType(event)  == AINPUT_EVENT_TYPE_KEY
              && AKeyEvent_getKeyCode(event) == AKEYCODE_BACK;

        // skip predispatch (all it does is send to the IME)
        if (!skip_predispatch && AInputQueue_preDispatchEvent(app->inputQueue, event)) {
            return;
        }

        int32_t handled = 0;
        if (app->onInputEvent != NULL) handled = app->onInputEvent(app, event);
        AInputQueue_finishEvent(app->inputQueue, event, handled);
    } else {
        LOGE("Failure reading next input event: %s\n", strerror(errno));
    }
}


void android_main(android_app* state) {
	app_dummy();
	state->inputPollSource.process = &process_input;

    signal(SIGSEGV, sighandler);
    signal(SIGFPE, 	sighandler);

    state->onAppCmd = &handle_cmd;
	state->onInputEvent = &handle_input;
	gApp = state;

	remoteDebugInitialize();
	initializeFileSystem();
	initSensors();
	lifecycle::create();


	bool fullyActive = false;
	while (true) {
		int ident, fdesc, events;
		android_poll_source* source;
		while (true)
		{
        	ident = ALooper_pollOnce(1, &fdesc, &events, (void**) &source);
			if(ident <= 0)
				break;
			if (source)
				source->process(state, source);

			processSensors(ident);

			if (state->destroyRequested) {
				RLOGI("%s", "Native Activity Was Fully Destroyed!");
				return;
			}
		}

		if (!fullyActive && gAppState.fullyActive())
			gameInitialize(context->GetScreenWidth(), context->GetScreenHeight());

		if (gAppState.fullyActive())
			gameStep(context);

		fullyActive = gAppState.fullyActive();
	}
}
