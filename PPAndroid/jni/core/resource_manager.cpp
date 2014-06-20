/*
 * resource_manager.cpp
 *
 *  Created on: Jun 18, 2014
 *      Author: Gustav
 */
#include "path.h"
#include "hash.h"
#include "assert.h"
#include <string>
#include "image_loader.h"
#include <cstdio>
#include <cstring>
#include "resource_manager.h"
#include <GLES2/gl2.h>
#include "asset_helper.h"

template<typename T> const HashID GetHash();

#define REGISTER_TYPE_HASH(type, name) \
    template<>const HashID GetHash<type>(){return bytesHash((name), strlen(name), 0);}

REGISTER_TYPE_HASH(Font, "Font");
REGISTER_TYPE_HASH(Texture, "Texture");

#define NULL_HANDLE ((Handle){ 0, 0, nullptr })

static Handle* handles;
static size_t handlesLength;
static std::string resourcePath;


void contentInitialize(size_t numResources, std::string resourceFolder)
{
	handles = new Handle[numResources];
	handlesLength = numResources;
	resourcePath = resourceFolder;

	for(int i = 0; i < handlesLength; i++)
	{
		handles[i] = NULL_HANDLE;
	}
}

void contentTerminate()
{
	for(int i = 0; i < handlesLength; i++)
	{
		if(handles[i].item != nullptr)
		{
			contentUnloadHandle(&handles[i]);
		}
	}
}

bool contentIsPathLoaded(const char* path)
{
	HashID id = bytesHash(path, strlen(path), 0);
	return contentIsHashLoaded(id);
}

bool contentIsHashLoaded(HashID id)
{
    for(int i = 0; i < handlesLength; i++)
	{
		if(handles[i].hashID == id)
			return true;
	}
	return false;
}

static Handle* contentAddItem(HashID id, HashID type, void* item)
{
    for(int i = 0; i < handlesLength; i++)
	{
		if(handles[i].item == nullptr)
		{
			handles[i].hashID = id;
			handles[i].typeID = type;
			handles[i].item = item;
			return &handles[i];
		}
	}
    return nullptr;
}

static Handle* contentLoadFont(const char* path, HashID id)
{
	ExternalAsset fontAsset(path);
	ExternalAsset textureAsset(path::changeExtension(path, ".png"));
    auto texture = loadTexture(textureAsset.buffer, textureAsset.length);

    const size_t padding = sizeof(Texture) + sizeof(CharInfo*) + sizeof(size_t)*2;
    auto fontBuffer = new uint8_t[fontAsset.length + padding];
    memcpy(fontBuffer + padding, fontAsset.buffer, fontAsset.length);
    auto font = (Font*)fontBuffer;

    font->chars = (CharInfo*)(fontBuffer + sizeof(Font));
    font->charsLength = (fontAsset.length + padding - sizeof(Font))/sizeof(CharInfo);

    font->page = texture;
    font->defaultChar = '_';

//    LOGI("Font size: %f", font->size);
//    LOGI("Font base: %f", font->base);
//    LOGI("Font lineHeight: %f", font->lineHeight);
//
//    LOGI("Font page glName: %d", font->page.glName);
//    LOGI("Font page width: %d", font->page.width);
//    LOGI("Font page height: %d", font->page.height);
//
//    LOGI("Font charslength: %d", font->charsLength);
//    LOGI("Font defaultChar: %d", font->defaultChar);
//    for(int i = 0; i < font->charsLength; i++)
//    {
//        LOGI("Font first char advance: %f", font->chars[i].advance);
//        LOGI("Font first char offset: x = %f | y = %f", font->chars[i].offset.x, font->chars[i].offset.y);
//        LOGI("Font first char srcrect: [%f,%f,%f,%f]",
//                font->chars[i].srcRect.x,
//                font->chars[i].srcRect.y,
//                font->chars[i].srcRect.z,
//                font->chars[i].srcRect.w);
//        LOGI("Font first char textureCoords: [%f,%f,%f,%f]",
//                font->chars[i].textureCoords.x,
//                font->chars[i].textureCoords.y,
//                font->chars[i].textureCoords.z,
//                font->chars[i].textureCoords.w);
//    }

    return contentAddItem(id, GetHash<Font>(), (void*) font);
}

static Handle* contentLoadTexture(const char* path, HashID id)
{
	ExternalAsset asset(path);
    auto texture = loadTexture(asset.buffer, asset.length);
    auto texPtr = new Texture();
    *texPtr = texture;
    return contentAddItem(id, GetHash<Texture>(), (void*) texPtr);
}

static std::string toString(uint32_t value)
{
	char buf[16];
	sprintf(buf, "%u", value);
	return std::string(buf);
}

Handle* contentGetHandle(HashID id)
{
    for(int i = 0; i < handlesLength; i++)
	{
		if(handles[i].hashID == id)
			return &handles[i];
	}
    return nullptr;
}

Handle* contentLoad(const char* path)
{
	std::string hashPath = path::withoutExtension(path);
	HashID id = bytesHash(hashPath.c_str(), hashPath.size(), 0);
	std::string hashString = toString(id);
	std::string absPath = path::buildPath(resourcePath, hashString);

	auto handle = contentGetHandle(id);
	if(handle != nullptr)
		return handle;

	if (path::hasExtension(path, ".png"))
	{
		absPath += ".png";
		return contentLoadTexture(absPath.c_str(), id);
	} else if (path::hasExtension(path, ".fnt")) {
		absPath += ".fnt";
		return contentLoadFont(absPath.c_str(), id);
	}
	ASSERT(false, std::string(std::string("Can't load resource ") + std::string(path)).c_str());
	return nullptr;
}

static void obliterateTexture(Texture* texture)
{
	glDeleteTextures(1, &texture->glName);
}

static void obliterateFont(Font* font)
{
	glDeleteTextures(1, &font->page.glName);
}

bool contentUnloadPath(const char* path)
{
	std::string hashPath = path::withoutExtension(path);
	HashID id = bytesHash(hashPath.c_str(), hashPath.size(), 0);
    for(int i = 0; i < handlesLength; i++)
	{
		if(handles[i].hashID == id)
		{
			if(path::hasExtension(path, ".png"))
			{
				obliterateTexture(((Texture*)handles[i].item));
                delete (Texture*)handles[i].item;
			}
			else if(path::hasExtension(path, ".fnt"))
			{
				obliterateFont(((Font*)handles[i].item));
                delete (Font*)handles[i].item;
			}
			handles[i] = NULL_HANDLE;
			return true;
		}
	}
    return false;
}

bool contentUnloadHandle(Handle* handle)
{
	if(handle->item == nullptr)
		return false;
	if(handle->typeID == GetHash<Texture>())
	{
		obliterateTexture(((Texture*)handle->item));
        delete (Texture*)handle->item;
	}
	else if(handle->typeID == GetHash<Font>())
	{
        obliterateFont(((Font*)handle->item));
        delete (Font*)handle->item;
	}
	*handle = NULL_HANDLE;
	return true;
}

void contentUnloadAll()
{
	for(int i=0; i<handlesLength; i++)
	{
		contentUnloadHandle(&handles[i]);
	}
}
