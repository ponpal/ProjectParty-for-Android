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
#include "JNIHelper.h"
#include <stdio.h>
#include "assert.h"
#include <errno.h>

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
	AutoPtr(size_t size)
	{
		ptr = new T[size];
	}
	AutoPtr(T* _ptr) : ptr(_ptr) { }
	AutoPtr() : ptr(NULL) { }

	~AutoPtr()
	{
		if(ptr)
			free(ptr);
	}
};

struct ExternalAsset
{
	size_t 		length;
	uint8_t* 	buffer;

	ExternalAsset(std::string fileStr)
	{
		auto externalsDir = gApp->activity->externalDataPath;
		std::string dirStr(externalsDir);
		std::string filePath = dirStr + "/" + fileStr;

		LOGI("Trying to load file %s", filePath.c_str());
        auto file = fopen(filePath.c_str(), "r+");
        ASSERTF(file != NULL, "Couldn't open file. %s", filePath.c_str());


        fseek( file, 0L, SEEK_END);
        length = ftell(file);
        rewind(file);
        LOGI("ftell: %d", length);

        buffer = new uint8_t[length];

        fread(buffer, length, 1, file);
		fclose(file);
	}

	~ExternalAsset()
	{
		delete buffer;
	}
};

#endif /* ASSET_HELPER_H_ */
