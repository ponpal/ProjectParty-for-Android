/*
 * vibrator.h
 *
 *  Created on: Apr 22, 2014
 *      Author: Gustav
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_


#ifdef __ANDROID__
	#include "android_native_app_glue.h"
	extern android_app* gApp;
#endif

extern "C"
{
	typedef struct Resource
	{
		uint8_t* buffer;
		uint32_t length;
	} Resource;


	int platformVibrate(uint64_t milliseconds);
	void platformDisplayKeyboard(bool pShow);
	const char* platformGetInputBuffer();
	void platformSetOrientation(int orientation);

	uint32_t platformGetBroadcastAddress();
	uint32_t platformLanIP();
	const char* platformDeviceName();
	const char* platformExternalResourceDirectory();
	Resource platformLoadAbsolutePath(const char* name);
	Resource platformLoadResource(const char* path);
	Resource platformLoadInternalResource(const char* path);
	Resource platformLoadExternalResource(const char* path);
	void platformUnloadResource(Resource resource);
	void platformExit();
}

#endif /* VIBRATOR_H_ */
