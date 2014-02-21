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
#include <netinet/in.h>


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
		return 1;
	}

	return 0;
}

ndk_helper::GLContext* context;

bool isInitialized;
bool noRender;

jclass serviceClass;


extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    LOGI("JNI_OnLoad was called!");

    auto clazz = env->FindClass("projectparty/ppandroid/ControllerService");
    LOGI("Loaded class! %d ", (size_t)clazz);
    serviceClass = (jclass)env->NewGlobalRef(clazz);

    return JNI_VERSION_1_6;
}


void connectToService(android_app* app)
{

	auto env = app->activity->env;
	app->activity->vm->AttachCurrentThread( &env, NULL );

	LOGI("Attached thread to environment!");

	auto clazz = serviceClass;

	LOGI("Got a class %d ", (size_t)serviceClass);

	auto dummy = env->GetStaticFieldID(clazz, "toAccess", "I");

	LOGI("Got a dummy static int! %d", (size_t)dummy);

	auto fi = env->GetStaticFieldID(clazz, "instance", "Lprojectparty/ppandroid/ControllerService;");
	LOGI("Fetched field ID! %d", (size_t)fi);
	LOGI(":(");

	auto obj = env->GetStaticObjectField(clazz, fi);
	LOGI("Fetched the object!");

	fi = env->GetFieldID(clazz, "inBuffer", "Ljava/nio/ByteBuffer;");
	LOGI("Got the field id for inBuffer");

	auto inBufferObj = env->GetObjectField(obj, fi);
	LOGI("Got in Buffer! %d", (size_t)inBufferObj);

	fi = env->GetFieldID(clazz, "outBuffer", "Ljava/nio/ByteBuffer;");

	auto outBufferObj = env->GetObjectField(obj, fi);

	LOGI("Got out Buffer! %d", (size_t)outBufferObj);

	auto inBuffer  = env->GetDirectBufferAddress(inBufferObj);
	auto outBuffer = env->GetDirectBufferAddress(outBufferObj);

	LOGI("Got access to buffers!");

	auto recID  = env->GetMethodID(clazz, "receive", "()I");
	auto sendID = env->GetMethodID(clazz, "send", "(I)I");

	auto outPtr = (uint8_t*)outBuffer;

	union Value
	{
		uint32_t x;
		float y;
	};

	while(1)
	{
		uint16_t* sPtr = (uint16_t*)(outPtr);
		*sPtr = htons(13);

		outPtr[2] = 1;

		Value v;
		v.y = 1.0f;

		uint32_t* ptr = (uint32_t*)(&outPtr[3]);
		*(ptr++) = htonl(v.x);
		v.y = 1.0f;
		*(ptr++) = htonl(v.x);
		v.y = 1.0f;
		*(ptr++) = htonl(v.x);

		auto written = env->CallIntMethod(obj, sendID, 15);

		LOGI("SENDING %d bytes of data.", written);
	}
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
			initializeLuaCore(app);
			initLuaCall();
			isInitialized = true;
	        break;
		case APP_CMD_TERM_WINDOW:
			LOGI("Window terminated");
			context->Suspend();
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
			connectToService(app);

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

	LOGI("Rendering!");
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


		   render();
		}
	}
}

