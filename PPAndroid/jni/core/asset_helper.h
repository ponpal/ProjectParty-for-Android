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

#include <stdio.h>

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

#include "log.h"
struct ExternalAsset
{
	size_t 		length;
	uint8_t* 	buffer;
	FILE* 		file;

	ExternalAsset(std::string fileStr)
	{
		auto externalsDir = gApp->activity->externalDataPath;
		std::string dirStr(externalsDir);
		std::string filePath = dirStr + "/" + fileStr;

        file = fopen(filePath.c_str(), "r+");
		buffer = file->_p;
		length = file->_r;
	}

	ExternalAsset(std::string fileStr, const char* buf, uint32_t size)
	{
		auto externalsDir = gApp->activity->externalDataPath;
		std::string dirStr(externalsDir);
		std::string filePath = dirStr + "/" + fileStr;

        file = fopen(filePath.c_str(), "w+");
        if (file != NULL) {
        	fwrite(buf, sizeof(char), size, file);
        	fclose(file);
        } else {
            LOGE("Unable to write/create file: %s", filePath.c_str());
        }
        file = fopen(filePath.c_str(), "r+");

		buffer = file->_p;
		length = file->_r;
	}

	~ExternalAsset()
	{
		fflush(file);
		fclose(file);
	}
};

#endif /* ASSET_HELPER_H_ */
