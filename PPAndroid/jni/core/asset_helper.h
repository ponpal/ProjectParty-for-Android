/*
 * asset_helper.h
 *
 *  Created on: Feb 23, 2014
 *      Author: Lukas_2
 */

#ifndef ASSET_HELPER_H_
#define ASSET_HELPER_H_

#include "game.h"
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
	Asset(const char* fileName);

	~Asset();

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
	ExternalAsset(const std::string& filePath);
	~ExternalAsset();
};

#endif /* ASSET_HELPER_H_ */
