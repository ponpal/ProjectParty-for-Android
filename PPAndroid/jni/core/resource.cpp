/*
 * resource.cpp
 *
 *  Created on: 18 jul 2014
 *      Author: Lukas
 */

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include "external_libs/lodepng.h"
#include "types.h"
#include "assert.h"
#include "resources.h"
#include "platform.h"
#include "path.h"

static void loadImage(const char* path, uint8_t** data, uint32_t* width, uint32_t* height)
{
	auto resource = platformLoadResource(path);
	RLOGI("%s", "Loading png.");
	auto err = lodepng_decode32(data, width, height, resource.buffer, resource.length);
	ASSERTF(err == 0, "Failed to load png! ERROR: %s BUF: %d, LEN: %d",
			lodepng_error_text(err), (uint32_t)resource.buffer, resource.length);
	RLOGI("%s", "LOADED PNG");
}


Texture loadTexture(const char* path)
{
	uint8_t* data;
	uint32_t width, height;
	loadImage(path, &data, &width, &height);

	GLuint name;
	glGenTextures(1, &name);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	free(data);

	return (Texture) { name, width, height };
}

void unloadTexture(Texture tex)
{
	glDeleteTextures(1, &tex.glName);
}

Texture reloadTexture(const char* path, Texture tex)
{
	uint8_t* data;
	uint32_t width, height;
	loadImage(path, &data, &width, &height);

	glBindTexture(GL_TEXTURE_2D, tex.glName);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	tex.width = width;
	tex.height = height;

	return tex;
}

Font* loadFont(const char* path)
{
	Resource fontAsset = platformLoadResource(path);
	auto texture = loadTexture(path::changeExtension(path, ".png").c_str());

	const size_t padding = sizeof(Texture) + sizeof(CharInfo*) + sizeof(size_t)*2;
	auto fontBuffer = malloc(fontAsset.length + padding);
	memcpy(fontBuffer + padding, fontAsset.buffer, fontAsset.length);
	auto font = (Font*)fontBuffer;

	font->chars = (CharInfo*)(fontBuffer + sizeof(Font));
	font->charsLength = (fontAsset.length + padding - sizeof(Font))/sizeof(CharInfo);

	font->page = texture;
	font->defaultChar = '_';

	platformUnloadResource(fontAsset);
	return font;

	//    RLOGI("Font size: %f", font->size);
	//    RLOGI("Font base: %f", font->base);
	//    RLOGI("Font lineHeight: %f", font->lineHeight);
	//
	//    RLOGI("Font page glName: %d", font->page.glName);
	//    RLOGI("Font page width: %d", font->page.width);
	//    RLOGI("Font page height: %d", font->page.height);
	//
	//    RLOGI("Font charslength: %d", font->charsLength);
	//    RLOGI("Font defaultChar: %d", font->defaultChar);
	//    for(int i = 0; i < font->charsLength; i++)
	//    {
	//        RLOGI("Font first char advance: %f", font->chars[i].advance);
	//        RLOGI("Font first char offset: x = %f | y = %f", font->chars[i].offset.x, font->chars[i].offset.y);
	//        RLOGI("Font first char srcrect: [%f,%f,%f,%f]",
	//                font->chars[i].srcRect.x,
	//                font->chars[i].srcRect.y,
	//                font->chars[i].srcRect.z,
	//                font->chars[i].srcRect.w);
	//        RLOGI("Font first char textureCoords: [%f,%f,%f,%f]",
	//                font->chars[i].textureCoords.x,
	//                font->chars[i].textureCoords.y,
	//                font->chars[i].textureCoords.z,
	//                font->chars[i].textureCoords.w);
	//    }

}


void unloadFont(Font* font)
{
	unloadTexture(font->page);
	free(font);
}

Font* reloadFont(const char* path, Font* fnt)
{
	Texture texture = reloadTexture(path::changeExtension(path, ".png").c_str(), fnt->page);
	free(fnt);

	Resource fontAsset = platformLoadResource(path);
	const size_t padding = sizeof(Texture) + sizeof(CharInfo*) + sizeof(size_t)*2;
	auto fontBuffer = (uint8_t*)malloc(fontAsset.length + padding);
	memcpy(fontBuffer + padding, fontAsset.buffer, fontAsset.length);
	auto font = (Font*)fontBuffer;

	font->chars = (CharInfo*)(fontBuffer + sizeof(Font));
	font->charsLength = (fontAsset.length + padding - sizeof(Font))/sizeof(CharInfo);

	font->page = texture;
	font->defaultChar = '_';

	platformUnloadResource(fontAsset);
	return font;
}

