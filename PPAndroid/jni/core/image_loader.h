/*
 * image_loader.h
 *
 *  Created on: Feb 13, 2014
 *      Author: Lukas_2
 */

#ifndef IMAGE_LOADER_H_
#define IMAGE_LOADER_H_

#include "types.h"
#include <jni.h>

Texture loadTexture(const char* path);
//uint8_t* loadBinary(const char* path, size_t& size);


#endif /* IMAGE_LOADER_H_ */