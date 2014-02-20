/*
 * image_loader.cpp
 *
 *  Created on: Feb 13, 2014
 *      Author: Lukas_2
 */

#include <stdexcept>
#include <GLES2/gl2.h>
#include "image_loader.h"
#include "JNIHelper.h"

Texture loadTexture(const char* path)
{
	auto helper = ndk_helper::JNIHelper::GetInstance();
	int32_t ok = helper->LoadTexture(path);
	if(ok != -1)
		return Texture(ok);

	throw std::domain_error("Could not load texture!");
}
