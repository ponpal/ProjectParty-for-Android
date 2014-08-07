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

void platformSetOrientation(int orientation)
{
	auto env = gApp->activity->env;
	auto vm = gApp->activity->vm;
	auto obj = gApp->activity->clazz;

	vm->AttachCurrentThread( &env, NULL );

	auto clazz = env->GetObjectClass(obj);
	jmethodID orientationID = env->GetMethodID(clazz, "setOrientation", "(I)V");

	env->CallVoidMethod(obj, orientationID, orientation);
	vm->DetachCurrentThread();
}

void platformDisplayKeyboard(bool pShow)
{
    jint lResult;
    jint lFlags = 0;

    JavaVM* lJavaVM = gApp->activity->vm;
    JNIEnv* lJNIEnv = gApp->activity->env;

    JavaVMAttachArgs lJavaVMAttachArgs;
    lJavaVMAttachArgs.version = JNI_VERSION_1_6;
    lJavaVMAttachArgs.name = "NativeThread";
    lJavaVMAttachArgs.group = NULL;

    lResult=lJavaVM->AttachCurrentThread(&lJNIEnv,
&lJavaVMAttachArgs);
    if (lResult == JNI_ERR) {
        return;
    }

    // Retrieves NativeActivity.
    jobject lNativeActivity = gApp->activity->clazz;
    jclass ClassNativeActivity = lJNIEnv->GetObjectClass(lNativeActivity);

    // Retrieves Context.INPUT_METHOD_SERVICE.
    jclass ClassContext = lJNIEnv->FindClass("android/content/Context");
    jfieldID FieldINPUT_METHOD_SERVICE =
        lJNIEnv->GetStaticFieldID(ClassContext,
            "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
    jobject INPUT_METHOD_SERVICE =
        lJNIEnv->GetStaticObjectField(ClassContext,
            FieldINPUT_METHOD_SERVICE);
    //jniCheck(INPUT_METHOD_SERVICE);

    // Runs getSystemService(Context.INPUT_METHOD_SERVICE).
    jclass ClassInputMethodManager = lJNIEnv->FindClass(
        "android/view/inputmethod/InputMethodManager");
    jmethodID MethodGetSystemService = lJNIEnv->GetMethodID(
        ClassNativeActivity, "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject lInputMethodManager = lJNIEnv->CallObjectMethod(
        lNativeActivity, MethodGetSystemService,
        INPUT_METHOD_SERVICE);

    // Runs getWindow().getDecorView().
    jmethodID MethodGetWindow = lJNIEnv->GetMethodID(
        ClassNativeActivity, "getWindow",
        "()Landroid/view/Window;");
    jobject lWindow = lJNIEnv->CallObjectMethod(lNativeActivity,
        MethodGetWindow);
    jclass ClassWindow = lJNIEnv->FindClass(
        "android/view/Window");
    jmethodID MethodGetDecorView = lJNIEnv->GetMethodID(
        ClassWindow, "getDecorView", "()Landroid/view/View;");
    jobject lDecorView = lJNIEnv->CallObjectMethod(lWindow,
        MethodGetDecorView);

    if (pShow) {
        // Runs lInputMethodManager.showSoftInput(...).
        jmethodID MethodShowSoftInput = lJNIEnv->GetMethodID(
            ClassInputMethodManager, "showSoftInput",
            "(Landroid/view/View;I)Z");
        jboolean lResult = lJNIEnv->CallBooleanMethod(
            lInputMethodManager, MethodShowSoftInput,
            lDecorView, lFlags);
    } else {
        // Runs lWindow.getViewToken()
        jclass ClassView = lJNIEnv->FindClass(
            "android/view/View");
        jmethodID MethodGetWindowToken = lJNIEnv->GetMethodID(
            ClassView, "getWindowToken", "()Landroid/os/IBinder;");
        jobject lBinder = lJNIEnv->CallObjectMethod(lDecorView,
            MethodGetWindowToken);

        // lInputMethodManager.hideSoftInput(...).
        jmethodID MethodHideSoftInput = lJNIEnv->GetMethodID(
            ClassInputMethodManager, "hideSoftInputFromWindow",
            "(Landroid/os/IBinder;I)Z");
        jboolean lRes = lJNIEnv->CallBooleanMethod(
            lInputMethodManager, MethodHideSoftInput,
            lBinder, lFlags);
    }

    // Finished with the JVM.
    lJavaVM->DetachCurrentThread();
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

static void callJNIReturnString(const char* str, char* result)
{
	auto env = gApp->activity->env;
	auto vm = gApp->activity->vm;
	auto obj = gApp->activity->clazz;

	vm->AttachCurrentThread( &env, NULL );
	auto clazz = env->GetObjectClass(obj);
	jmethodID methodID = env->GetMethodID(clazz, str, "()[B");

	auto array = (jbyteArray)env->CallObjectMethod(obj, methodID);
	auto ptr   = env->GetByteArrayElements(array, 0);
	int len   = env->GetArrayLength(array);

	memcpy(result, ptr, len);
	result[len] = '\0';

	env->ReleaseByteArrayElements(array, ptr, JNI_ABORT);

	vm->DetachCurrentThread();
}

const char* platformDeviceName()
{
	static char deviceName[128];
	callJNIReturnString("getDeviceName", deviceName);
	return deviceName;
}
const char* platformGetInputBuffer()
{
	static char inputBuffer[64];
	callJNIReturnString("getInputBuffer", inputBuffer);
	return inputBuffer;
}

Resource platformLoadInternalResource(const char* path)
{
    RLOGI("Trying to internal load file %s", path);
	auto mgr  = gApp->activity->assetManager;
	auto asset = AAssetManager_open(mgr, path, AASSET_MODE_RANDOM);
	ASSERTF(asset != 0, "The asset %s does not exist", path);

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
    RLOGI("Trying to external load file %s", path);
	auto resourcePath = path::buildPath(gApp->activity->externalDataPath, path);
	return platformLoadAbsolutePath(resourcePath.c_str());
}

Resource platformLoadResource(const char* path)
{
	RLOGI("Opening resource %s", path);
	auto externalPath = path::buildPath(gApp->activity->externalDataPath, path);
	auto file = fopen(externalPath.c_str(), "r+");
	if(file == 0)
	{
		RLOGI("File is not external %s", path);
		return platformLoadInternalResource(path);
	}

	fseek( file, 0L, SEEK_END);
	auto length = ftell(file);
	rewind(file);
	auto buffer = new uint8_t[length];
	fread(buffer, length, 1, file);
	fclose(file);

	return (Resource) { buffer, (uint32_t)length };
}

Resource platformLoadAbsolutePath(const char* resourcePath)
{
    auto file = fopen(resourcePath, "r+");
    ASSERTF(file != 0, "Couldn't open file. %s", resourcePath);
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

