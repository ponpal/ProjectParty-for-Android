/*
 * image_loader.cpp
 *
 *  Created on: Feb 13, 2014
 *      Author: Lukas_2
 */

#include <GLES2/gl2.h>
#include "external_libs/lodepng.h"
#include "asset_helper.h"
#include "types.h"
#include "assert.h"

Texture loadTexture(uint8_t* buffer, uint32_t length)
{

	AutoPtr<uint8_t> pngData;

	uint32_t width, height;
	auto err = lodepng_decode32(&pngData.ptr, &width, &height, buffer, length);
	ASSERT(err == 0, "Failed to load png!");

	GLuint name;
	glGenTextures(1, &name);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pngData.ptr);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return Texture(name, width, height);
}
