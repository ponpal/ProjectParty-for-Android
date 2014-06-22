/*
 * vibrator.h
 *
 *  Created on: Apr 22, 2014
 *      Author: Gustav
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "resource_manager.h"

#ifdef __ANDROID__
#include "android_native_app_glue.h"
extern android_app* gApp;
#endif

extern "C" {
int platformVibrate(uint64_t milliseconds);
uint32_t platformGetBroadcastAddress();
const char* platformExternalResourceDirectory();
Resource platformLoadAbsolutePath(const char* name);
Resource platformLoadInternalResource(const char* path);
Resource platformLoadExternalResource(const char* path);
void platformUnloadResource(Resource resource);
void platformExit();
}
#endif /* VIBRATOR_H_ */
