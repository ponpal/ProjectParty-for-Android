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
#include "JNIHelper.h"
#include "android_helper.h"


jclass gNetworkServiceClass;
jobject gNetworkServiceObject;
jmethodID sendID, receiveID, isAliveID, connectID, disconnectID, shutdownID, reconnectID;

void networkServiceClass(jclass clazz)
{
	gNetworkServiceClass = clazz;
}

Network* networkInitialize(android_app* app)
{
	gApp = app;

	auto env = app->activity->env;
	app->activity->vm->AttachCurrentThread( &env, NULL );
	auto clazz = gNetworkServiceClass;
	auto dummy = env->GetStaticFieldID(clazz, "toAccess", "I");
	auto fi = env->GetStaticFieldID(clazz, "instance", "Lprojectparty/ppandroid/services/ControllerService;");
	auto obj = env->GetStaticObjectField(clazz, fi);

	gNetworkServiceObject = env->NewGlobalRef(obj);

	sendID       = env->GetMethodID(clazz, "send", "(I)I");
	receiveID    = env->GetMethodID(clazz, "receive", "()I");
	isAliveID    = env->GetMethodID(clazz, "isAlive", "()I");
	connectID    = env->GetMethodID(clazz, "connect", "()I");
	reconnectID  = env->GetMethodID(clazz, "reconnect", "()I");
	disconnectID = env->GetMethodID(clazz, "disconnect", "()I");
	shutdownID   = env->GetMethodID(clazz, "shutdown", "()I");

	fi = env->GetFieldID(clazz, "inBuffer", "Ljava/nio/ByteBuffer;");

	auto inBufferObj = env->GetObjectField(obj, fi);

	fi = env->GetFieldID(clazz, "outBuffer", "Ljava/nio/ByteBuffer;");

	auto outBufferObj = env->GetObjectField(obj, fi);


	auto network = new Network();
	network->in_  = new Buffer();
	network->out = new Buffer();

	network->in_->base  = network->in_->ptr  = (uint8_t*)env->GetDirectBufferAddress(inBufferObj);
	network->out->base = network->out->ptr = (uint8_t*)env->GetDirectBufferAddress(outBufferObj);

	network->in_->length  = env->GetDirectBufferCapacity(inBufferObj);
	network->out->length = env->GetDirectBufferCapacity(outBufferObj);

	app->activity->vm->DetachCurrentThread();

	return network;
}

void networkDelete(Network* network)
{
	delete network->in_;
	delete network->out;
	delete network;
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

	if (result == -1) {
		LOGI("network error");
	}

	return result;
}

int networkIsAlive(Network* network)
{
	auto env = gApp->activity->env;
	gApp->activity->vm->AttachCurrentThread( &env, NULL );
	auto result = env->CallIntMethod(gNetworkServiceObject, isAliveID);
	gApp->activity->vm->DetachCurrentThread();
	return result;
}

int networkConnect(Network* network)
{
	LOGI("Connecting to network.");
	auto env = gApp->activity->env;
	gApp->activity->vm->AttachCurrentThread( &env, NULL );
	auto result = env->CallIntMethod(gNetworkServiceObject, connectID);
	gApp->activity->vm->DetachCurrentThread();
	if (result == 1)
        LOGI("Connected to network.");
	else if (result == 0) {
		LOGE("NETWORK FAILURE");
		ANativeActivity_finish(gApp->activity);
	}
	return result;
}

int networkDisconnect(Network* network)
{
	LOGI("Disconnecting from network.");
	auto env = gApp->activity->env;
	gApp->activity->vm->AttachCurrentThread( &env, NULL );
	auto result = env->CallIntMethod(gNetworkServiceObject, disconnectID);
	gApp->activity->vm->DetachCurrentThread();
	if (result == 1)
        LOGI("Disconnected from network.");
	return result;
}

int networkReconnect(Network* network)
{
    LOGI("Reconnecting to network.");
	auto env = gApp->activity->env;
	gApp->activity->vm->AttachCurrentThread( &env, NULL );
	auto result = env->CallIntMethod(gNetworkServiceObject, reconnectID);
	gApp->activity->vm->DetachCurrentThread();
	if (result == 1)
        LOGI("Reconnected to network.");
	return result;
}

int networkShutdown(Network* network)
{
	LOGI("Shutting down network.");
	auto env = gApp->activity->env;
	gApp->activity->vm->AttachCurrentThread( &env, NULL );
	auto result = env->CallIntMethod(gNetworkServiceObject, shutdownID);
	gApp->activity->vm->DetachCurrentThread();
	if (result == 1)
        LOGI("Shut down network.");
	return result;
}


