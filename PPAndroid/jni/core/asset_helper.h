/*
 * asset_helper.h
 *
 *  Created on: Feb 23, 2014
 *      Author: Lukas_2
 */

#ifndef ASSET_HELPER_H_
#define ASSET_HELPER_H_

#include <android/asset_manager.h>
#include "android_helper.h"
#include "stdlib.h"

struct Asset
{
	size_t length;
	uint8_t* buffer;
	AAsset* asset;

	Asset(const char* fileName)
	{
		auto mgr  = gApp->activity->assetManager;
		asset = AAssetManager_open(mgr, fileName, AASSET_MODE_RANDOM);
		length = AAsset_getLength(asset);
		buffer = (uint8_t*)AAsset_getBuffer(asset);

	}

	~Asset()
	{
		AAsset_close(asset);
	}
};

template< typename T>
struct AutoPtr
{
	T* ptr;
	AutoPtr(T* _ptr) : ptr(_ptr) { }
	AutoPtr() : ptr(NULL) { }

	~AutoPtr()
	{
		if(ptr)
			free(ptr);
	}
};

#endif /* ASSET_HELPER_H_ */
