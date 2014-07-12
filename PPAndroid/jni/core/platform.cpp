/*
 * vibrator.cpp
 *
 *  Created on: Apr 22, 2014
 *      Author: Gustav
 */

#ifdef __ANDROID__

#include <jni.h>
#include "platform.h"
#include "JNIHelper.h"
#include "game.h"
#include <android/asset_manager.h>
#include "stdlib.h"
#include <stdio.h>
#include "assert.h"
#include <errno.h>
#include "path.h"

android_app* gApp;

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

uint32_t platformLanIP()
{
	auto env = gApp->activity->env;
	auto vm = gApp->activity->vm;
	auto obj = gApp->activity->clazz;

	vm->AttachCurrentThread( &env, NULL );

	auto clazz = env->GetObjectClass(obj);
	jmethodID methodID = env->GetMethodID(clazz, "getLanIp", "()I");
	auto result = env->CallIntMethod(obj, methodID);
	vm->DetachCurrentThread();
	return result;
}

static char deviceName[128];
const char* platformDeviceName()
{
	auto env = gApp->activity->env;
	auto vm = gApp->activity->vm;
	auto obj = gApp->activity->clazz;

	vm->AttachCurrentThread( &env, NULL );


	auto clazz = env->GetObjectClass(obj);
	jmethodID methodID = env->GetMethodID(clazz, "getDeviceName", "()[B");
	LOGI("Method : %d", (uint32_t)methodID);

	auto array = (jbyteArray)env->CallObjectMethod(obj, methodID);
	auto ptr   = env->GetByteArrayElements(array, 0);
	int len   = env->GetArrayLength(array);

	LOGI("Exit");

	memcpy(deviceName, ptr, len);
	deviceName[len] = '\0';

	LOGI("DName %s", deviceName);

	vm->DetachCurrentThread();
	return deviceName;
}

Resource platformLoadInternalResource(const char* path)
{
    RLOGI("Trying to load file %s", path);
	auto mgr  = gApp->activity->assetManager;
	auto asset = AAssetManager_open(mgr, path, AASSET_MODE_RANDOM);
	auto length = AAsset_getLength(asset);
	auto buffer = new uint8_t[length];
	auto err = AAsset_read(asset, buffer, length);
	ASSERTF(err >= 0, "Failed to load internal resource %s. Error was: %d", path, err);
	AAsset_close(asset);

	return (Resource){ buffer, (uint32_t)length };
}

const char* platformExternalResourceDirectory()
{
	return gApp->activity->externalDataPath;
}

Resource platformLoadExternalResource(const char* path)
{
    RLOGI("Trying to load file %s", path);
	auto resourcePath = path::buildPath(gApp->activity->externalDataPath, path);
	return platformLoadAbsolutePath(resourcePath.c_str());
}

Resource platformLoadAbsolutePath(const char* resourcePath)
{
    auto file = fopen(resourcePath, "r+");
    ASSERTF(file != NULL, "Couldn't open file. %s", resourcePath);
    fseek( file, 0L, SEEK_END);
    auto length = ftell(file);
    rewind(file);
    auto buffer = new uint8_t[length];
    fread(buffer, length, 1, file);
    fclose(file);
    RLOGI("%s", "Successfully loaded!");
    return (Resource){ buffer, (uint32_t)length };
}

void platformUnloadResource(Resource resource)
{
	delete [] resource.buffer;
}

void platformExit()
{
	ANativeActivity_finish(gApp->activity);
}

#endif

