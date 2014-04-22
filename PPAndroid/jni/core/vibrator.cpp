/*
 * vibrator.cpp
 *
 *  Created on: Apr 22, 2014
 *      Author: Gustav
 */

#include <jni.h>
#include "android_helper.h"
#include "vibrator.h"

int vibrate(uint64_t milliseconds)
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
