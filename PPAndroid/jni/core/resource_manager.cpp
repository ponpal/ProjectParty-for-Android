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
#include "platform.h"
#include "remote_log.h"

template<typename T> const HashID GetHash();

#define REGISTER_TYPE_HASH(type, name) \
    template<>const HashID GetHash<type>(){return bytesHash((name), strlen(name), 0);}

REGISTER_TYPE_HASH(Font, "Font");
REGISTER_TYPE_HASH(Texture, "Texture");

#define NULL_HANDLE ((Handle){ 0, 0, nullptr })

#define LOCAL_RESOURCE_PATH

static Resource loadLocal(ResourceManager* resources, const char* path)
{
	return platformLoadInternalResource(path);
}

static Resource loadExternal(ResourceManager* resources, const char* path)
{
	RLOGI("%s", path);
	return platformLoadExternalResource(path::buildPath(resources->resourceDir, path).c_str());
}

ResourceManager* resourceCreateLocal(size_t numResources)
{
	auto resources = new ResourceManager();
	resources->handles = new Handle[numResources];
	resources->handlesLength = numResources;
	resources->resourceDir = nullptr;
	resources->loadResource = &loadLocal;

	for(int i = 0; i < resources->handlesLength; i++)
	{
		resources->handles[i] = NULL_HANDLE;
	}
	return resources;
}

ResourceManager* resourceCreateNetwork(size_t numResources, const char* resourceFolder)
{
	auto resources = new ResourceManager();
	resources->handles = new Handle[numResources];
	resources->handlesLength = numResources;
	resources->resourceDir = resourceFolder;
	resources->loadResource = &loadExternal;

	for(int i = 0; i < resources->handlesLength; i++)
	{
		resources->handles[i] = NULL_HANDLE;
	}

	return resources;
}

void resourceDestroy(ResourceManager* resources)
{
	for(int i = 0; i < resources->handlesLength; i++)
	{
		if(resources->handles[i].item != nullptr)
		{
			resourceUnloadHandle(resources, &resources->handles[i]);
		}
	}
}

bool resourceIsPathLoaded(ResourceManager* resources, const char* path)
{
	HashID id = bytesHash(path, strlen(path), 0);
	return resourceIsHashLoaded(resources, id);
}

bool resourceIsHashLoaded(ResourceManager* resources, HashID id)
{
    for(int i = 0; i < resources->handlesLength; i++)
	{
		if(resources->handles[i].hashID == id)
			return true;
	}
	return false;
}

static Handle* resourceAddItem(ResourceManager* resources, HashID id, HashID type, void* item)
{
    for(int i = 0; i < resources->handlesLength; i++)
	{
		if(resources->handles[i].item == nullptr)
		{
			resources->handles[i].hashID = id;
			resources->handles[i].typeID = type;
			resources->handles[i].item = item;
			return &resources->handles[i];
		}
	}
    return nullptr;
}

static Handle* resourceLoadFont(ResourceManager* resources, const char* path, HashID id)
{
	Resource fontAsset = resources->loadResource(resources, path);
	Resource textureAsset = resources->loadResource(resources, path::changeExtension(path, ".png").c_str());
    auto texture = loadTexture(textureAsset.buffer, textureAsset.length);

    const size_t padding = sizeof(Texture) + sizeof(CharInfo*) + sizeof(size_t)*2;
    auto fontBuffer = new uint8_t[fontAsset.length + padding];
    memcpy(fontBuffer + padding, fontAsset.buffer, fontAsset.length);
    auto font = (Font*)fontBuffer;

    font->chars = (CharInfo*)(fontBuffer + sizeof(Font));
    font->charsLength = (fontAsset.length + padding - sizeof(Font))/sizeof(CharInfo);

    font->page = texture;
    font->defaultChar = '_';

    platformUnloadResource(fontAsset);
    platformUnloadResource(textureAsset);
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
    return resourceAddItem(resources, id, GetHash<Font>(), (void*) font);

}

static Handle* resourceLoadTexture(ResourceManager* resources, const char* path, HashID id)
{
	Resource asset = resources->loadResource(resources, path);
    auto texture = loadTexture(asset.buffer, asset.length);
    auto texPtr = new Texture();
    *texPtr = texture;
    platformUnloadResource(asset);
    return resourceAddItem(resources, id, GetHash<Texture>(), (void*) texPtr);
}

static std::string toString(uint32_t value)
{
	char buf[16];
	sprintf(buf, "%u", value);
	return std::string(buf);
}

Handle* resourceGetHandle(ResourceManager* resources, HashID id)
{
    for(int i = 0; i < resources->handlesLength; i++)
	{
		if(resources->handles[i].hashID == id)
			return &resources->handles[i];
	}
    return nullptr;
}

Handle* resourceLoad(ResourceManager* resources, const char* path)
{
	std::string hashPath = path::withoutExtension(path);
	HashID id = bytesHash(hashPath.c_str(), hashPath.size(), 0);
	std::string hashString = toString(id);
	std::string absPath = hashString;

	auto handle = resourceGetHandle(resources, id);
	if(handle != nullptr)
		return handle;

	if (path::hasExtension(path, ".png"))
	{
		absPath += ".png";
		return resourceLoadTexture(resources, absPath.c_str(), id);
	} else if (path::hasExtension(path, ".fnt")) {
		absPath += ".fnt";
		return resourceLoadFont(resources, absPath.c_str(), id);
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

bool resourceUnloadPath(ResourceManager* resources, const char* path)
{
	std::string hashPath = path::withoutExtension(path);
	HashID id = bytesHash(hashPath.c_str(), hashPath.size(), 0);
    for(int i = 0; i < resources->handlesLength; i++)
	{
		if(resources->handles[i].hashID == id)
		{
			if(path::hasExtension(path, ".png"))
			{
				obliterateTexture(((Texture*)resources->handles[i].item));
                delete (Texture*)resources->handles[i].item;
			}
			else if(path::hasExtension(path, ".fnt"))
			{
				obliterateFont(((Font*)resources->handles[i].item));
                delete [] (uint8_t*)resources->handles[i].item;
			}
			resources->handles[i] = NULL_HANDLE;
			return true;
		}
	}
    return false;
}

bool resourceUnloadHandle(ResourceManager* resources, Handle* handle)
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
        delete [] (uint8_t*)handle->item;
	}
	*handle = NULL_HANDLE;
	return true;
}

void resourceUnloadAll(ResourceManager* resources)
{
	for(int i=0; i<resources->handlesLength; i++)
	{
		resourceUnloadHandle(resources, &resources->handles[i]);
	}
}
