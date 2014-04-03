/*
 * main.cpp
 *
 *  Created on: Feb 15, 2014
 *      Author: Lukas_2
 */

#include "main.h"
#include "sys/stat.h"
#include "errno.h"
#include "core/sensor.h"

ndk_helper::GLContext* context;
ndk_helper::DragDetector drag_detector;
ndk_helper::PinchDetector pinch_detector;
TapDetector tap_detector;

static int32_t handle_input(android_app* app, AInputEvent* event) {
	auto type = AInputEvent_getType(event);

	if (type == AINPUT_EVENT_TYPE_MOTION) {
		GESTURE_STATE tapState = tapDetect(&tap_detector, event);
		if (tapState == MY_GESTURE_STATE_ACTION) {
			luaOnTap(tap_detector.x,
					context->GetScreenHeight() - tap_detector.y);
		}
		GESTURE_STATE dragState = drag_detector.Detect(event);
		//Handle drag state
		if (dragState & ndk_helper::GESTURE_STATE_START) {
			//Otherwise, start dragging
			ndk_helper::Vec2 v;
			drag_detector.GetPointer(v);
			float x, y;
			v.Value(x, y);
			luaOnDragBegin(x, context->GetScreenHeight() - y);
		} else if (dragState & ndk_helper::GESTURE_STATE_MOVE) {
			ndk_helper::Vec2 v;
			drag_detector.GetPointer(v);
			float x, y;
			v.Value(x, y);
			luaOnDrag(x, context->GetScreenHeight() - y);
		} else if (dragState & ndk_helper::GESTURE_STATE_END) {
			ndk_helper::Vec2 v;
			drag_detector.GetPointer(v);
			float x, y;
			v.Value(x, y);
			luaOnDragEnd(x, context->GetScreenHeight() - y);
		}

		ndk_helper::GESTURE_STATE pinchState = pinch_detector.Detect(
				event);

		if (pinchState & ndk_helper::GESTURE_STATE_START) {
			//Start new pinch
			LOGI("Pinch start");
			ndk_helper::Vec2 v1;
			ndk_helper::Vec2 v2;
			float x1, y1, x2, y2;
			pinch_detector.GetPointers(v1, v2);
			v1.Value(x1, y1);
			v2.Value(x2, y2);
			luaOnPinchBegin(x1, y1, x2, y2);
		} else if (pinchState & ndk_helper::GESTURE_STATE_MOVE) {
			//Multi touch
			//Start new pinch
			LOGI("Pinch");
			ndk_helper::Vec2 v1;
			ndk_helper::Vec2 v2;
			float x1, y1, x2, y2;
			pinch_detector.GetPointers(v1, v2);
			v1.Value(x1, y1);
			v2.Value(x2, y2);
			luaOnPinch(x1, y1, x2, y2);
		}

		size_t pointerCount = AMotionEvent_getPointerCount(event);
		for (size_t i = 0; i < pointerCount; i++) {
			luaOnTouch(AMotionEvent_getX(event, i), AMotionEvent_getY(event, i),
					i);
		}
		return 1;
	} else if (type == AINPUT_EVENT_TYPE_KEY) {
		LOGI("Received key event: %d", AKeyEvent_getKeyCode(event));
		return 0;
	}

	return 0;
}

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env;
	if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
		return -1;
	}
	LOGI("JNI_OnLoad was called!");

	auto tmp = env->FindClass(
			"projectparty/ppandroid/services/ControllerService");
	LOGI("Loaded class! %d ", (size_t) tmp);
	auto clazz = (jclass) env->NewGlobalRef(tmp);

	networkServiceClass(clazz);

	return JNI_VERSION_1_6;
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
			(1000L) * 1000);
}

void pauseSensors() {
	ASensorEventQueue_disableSensor(gSensorEventQueue, gAccelerometerSensor);
}

void processSensors(int32_t id) {
	if (id == LOOPER_ID_USER) {
		ASensorEvent event;
		while (ASensorEventQueue_getEvents(gSensorEventQueue, &event, 1) <= 0) {
			switch (event.type) {
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

namespace lifecycle {
void create() {
	context = ndk_helper::GLContext::GetInstance();

	gAppState.isStarted = false;
	gAppState.isResumed = false;
	gAppState.isFocused = false;
	gAppState.hasSurface = false;
	gAppState.wasStopped = false;

	gameInitialize(gApp);
}

void start() {
	if (gAppState.wasStopped)
		restart();
	else
		freshStart();
}

void restart() {
	LOGI("App was restarted!");
	gAppState.isStarted = true;

	gameRestart();
}

void freshStart() {
	LOGI("App was started!");
	gAppState.isStarted = true;

	gameStart();
}

void resume() {
	LOGI("App was resumed!");
	gAppState.isResumed = true;

	gameResume();
}

void pause() {
	LOGI("App was paused!");
	gAppState.isResumed = false;
	gAppState.isFocused = false;

	gamePause();
}

void stop() {
	LOGI("App was stopped!");

	gAppState.wasStopped = true;
	gAppState.isStarted = false;

	gameStop();
}

void destroy() {
	LOGI("App was destroyed!");
	gameTerminate();
}

//Focus Events
void gainedFocus() {
	LOGI("App gained focus");
	gAppState.isFocused = true;
	resumeSensors();
}

void lostFocus() {
	LOGI("App lost focus");
	gAppState.isFocused = false;
	pauseSensors();
}

//EGL calls
void surfaceCreated() {
	LOGI("Surface created graphics resources can be safely loaded.");
	gAppState.hasSurface = true;

	context->Init(gApp->window);
	gGame->screen->width = context->GetScreenWidth();
	gGame->screen->height = context->GetScreenHeight();
	gameSurfaceCreated();

}

void surfaceDestroyed() {
	LOGI("Surface Destroyed");
	gAppState.hasSurface = false;

	context->Invalidate();

	gameSurfaceDestroyed();
}

void surfaceChanged() {
	LOGI("Surface Changed");
	gGame->screen->width = context->GetScreenWidth();
	gGame->screen->height = context->GetScreenHeight();
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
	LOGI("EXTERNALSDIR: %s", d.c_str());
	d.erase(d.size() - 6, 6);
	LOGI("ERASED: %s", d.c_str());

	int err = mkdir(d.c_str(), 0770);
	if (err != 0 && errno != 17)
		LOGI("Error is: %d %s", errno, strerror(errno));

	err = mkdir(externalsDir, 0770);
	if (err != 0 && errno != 17)
		LOGI("Error is: %d %s", errno, strerror(errno));
}

void android_main(android_app* state) {
	app_dummy();
	state->onAppCmd = &handle_cmd;
	state->onInputEvent = &handle_input;
	gApp = state;

	initializeFileSystem();
	initSensors();
	lifecycle::create();

	while (1) {
		int ident, fdesc, events;
		android_poll_source* source;

		while (true)
		{
			LOGE("going to poll");
			ident = ALooper_pollOnce(1, &fdesc, &events, (void**) &source);
			LOGE("HEJH� android_main while");
			if(ident <= 0)
				break;
			if (source)
			{
				LOGE("android_main ifnr1 enter");
				source->process(state, source);
				LOGE("android_main ifnr1 exit");
			}

			LOGE("about to process sensors");
			processSensors(ident);
			LOGE("processing sensors is hard work, work");

			if (state->destroyRequested) {
				LOGI("Native Activity Was Fully Destroyed!");
				return;
			}
		}
		LOGE("android_main after loop");

		if (gAppState.fullyActive()) {
			LOGE("android_main ifnr3");
			gGame->screen->width = context->GetScreenWidth();
			gGame->screen->height = context->GetScreenHeight();
			gameStep(context);
		}
	}

}



