/*
 * vibrator.cpp
 *
 *  Created on: Apr 22, 2014
 *      Author: Gustav
 */

#include <jni.h>
#include "android_helper.h"
#include "android_platform.h"
#include "JNIHelper.h"

int platformVibrate(uint64_t milliseconds)
{
	auto env = gApp->activity->env;
	auto vm = gApp->activity->vm;
	auto obj = gApp->activity->clazz;

    vm->AttachCurrentThread( &env, NULL );

	auto clazz = env->GetObjectClass(obj);
	jmethodID vibrateID = env->GetMethodID(clazz, "vibrate", "(J)I");
    auto result = env->CallIntMethod(obj, vibrateID, milliseconds);
    vm->DetachCurrentThread();
    return result;
}

uint16_t platformGetPort()
{
	auto env = gApp->activity->env;
	auto vm = gApp->activity->vm;
	auto obj = gApp->activity->clazz;

    vm->AttachCurrentThread( &env, NULL );

	auto clazz = env->GetObjectClass(obj);
	jmethodID portID = env->GetMethodID(clazz, "getPort", "()I");
    auto result = env->CallIntMethod(obj, portID);
    vm->DetachCurrentThread();
    return result;
}

uint32_t platformGetIP()
{
	auto env = gApp->activity->env;
	auto vm = gApp->activity->vm;
	auto obj = gApp->activity->clazz;

    vm->AttachCurrentThread( &env, NULL );

	auto clazz = env->GetObjectClass(obj);
	jmethodID ipID = env->GetMethodID(clazz, "getIP", "()I");
    auto result = env->CallIntMethod(obj, ipID);
    vm->DetachCurrentThread();
    return result;
}

uint32_t platformGetBroadcastAddress()
{
	auto env = gApp->activity->env;
	auto vm = gApp->activity->vm;
	auto obj = gApp->activity->clazz;

    vm->AttachCurrentThread( &env, NULL );

	auto clazz = env->GetObjectClass(obj);
	jmethodID broadcastID = env->GetMethodID(clazz, "getBroadcastAddress", "()I");
    auto result = env->CallIntMethod(obj, broadcastID);
    vm->DetachCurrentThread();
    return result;
}
