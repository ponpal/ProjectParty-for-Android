/*
 * network.cpp
 *
 *  Created on: Feb 21, 2014
 *      Author: Lukas_2
 */

#include "network.h"
#include <android_native_app_glue.h>
#include <cstdlib>
#include <cstring>
#include "NDKHelper.h"
#include "android_helper.h"


jclass gNetworkServiceClass;
jobject gNetworkServiceObject;
jmethodID sendID, receiveID, isAliveID, reconnectID;

void networkServiceClass(jclass clazz)
{
	gNetworkServiceClass = clazz;
}

Network* networkInitialize(android_app* app)
{
	gApp = app;

	auto env = app->activity->env;
	app->activity->vm->AttachCurrentThread( &env, NULL );

	LOGI("Attached thread to environment!");
	auto clazz = gNetworkServiceClass;

	LOGI("Got a class %d ", (size_t)clazz);

	auto dummy = env->GetStaticFieldID(clazz, "toAccess", "I");

	LOGI("Got a dummy static int! %d", (size_t)dummy);

	auto fi = env->GetStaticFieldID(clazz, "instance", "Lprojectparty/ppandroid/ControllerService;");
	LOGI("Fetched field ID! %d", (size_t)fi);
	LOGI(":(");

	auto obj = env->GetStaticObjectField(clazz, fi);
	gNetworkServiceObject = env->NewGlobalRef(obj);
	LOGI("Fetched the object!");

	receiveID   = env->GetMethodID(clazz, "receive", "()I");
	sendID      = env->GetMethodID(clazz, "send", "(I)I");
	isAliveID   = env->GetMethodID(clazz, "isAlive", "()Z");
	reconnectID = env->GetMethodID(clazz, "reconnect", "()Z");

	fi = env->GetFieldID(clazz, "inBuffer", "Ljava/nio/ByteBuffer;");
	LOGI("Got the field id for inBuffer");

	auto inBufferObj = env->GetObjectField(obj, fi);
	LOGI("Got in Buffer! %d", (size_t)inBufferObj);

	fi = env->GetFieldID(clazz, "outBuffer", "Ljava/nio/ByteBuffer;");

	auto outBufferObj = env->GetObjectField(obj, fi);

	LOGI("Got out Buffer! %d", (size_t)outBufferObj);

	auto network = new Network();
	network->in_  = new Buffer();
	network->out = new Buffer();

	network->in_->base  = network->in_->ptr  = (uint8_t*)env->GetDirectBufferAddress(inBufferObj);
	network->out->base = network->out->ptr = (uint8_t*)env->GetDirectBufferAddress(outBufferObj);

	network->in_->length  = env->GetDirectBufferCapacity(inBufferObj);
	network->out->length = env->GetDirectBufferCapacity(outBufferObj);

	LOGI("Got access to buffers!");
	app->activity->vm->DetachCurrentThread();

	return network;
}

int networkSend(Network* network)
{
	Buffer* out = network->out;

	ptrdiff_t length = out->ptr - out->base;
	if(length == 0) return 1; //Nothing to send.

	auto env = gApp->activity->env;
	gApp->activity->vm->AttachCurrentThread( &env, NULL );

	auto result = env->CallIntMethod(gNetworkServiceObject, sendID, length);

	if(result != length) {
		LOGW("Network did not send enough bytes! Sent: %d, Excpected: %d",
		      (uint32_t)result, (uint32_t)length);
	}

	gApp->activity->vm->DetachCurrentThread();

	out->ptr = out->base;
	return 1;
}
int networkReceive(Network* network)
{
	auto in = network->in_;
	in->ptr = in->base;

	auto env = gApp->activity->env;
	gApp->activity->vm->AttachCurrentThread( &env, NULL );

	auto result = env->CallIntMethod(gNetworkServiceObject, receiveID);
	in->length = result;

	gApp->activity->vm->DetachCurrentThread();

	return result;
}

int networkIsAlive(Network* network)
{
//	auto env = app->activity->env;
//	app->activity->vm->AttachCurrentThread( &env, NULL );
//	auto result = env->CallBooleanMethod(gNetworkServiceObject, isAliveID);
//	app->activity->vm->DetachCurrentThread();
//
//	return result;

	return 1; //TBI
}

int networkReconnect(Network* network)
{
//	auto env = app->activity->env;
//	app->activity->vm->AttachCurrentThread( &env, NULL );
//	auto result = env->CallBooleanMethod(gNetworkServiceObject, isAliveID);
//	app->activity->vm->DetachCurrentThread();
//
//	return result;

	return 0; //TBI
}
