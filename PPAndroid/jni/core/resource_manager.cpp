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
	ASSERT(false, "Font loading is not yet implemented.");
	return nullptr;
}

static Handle* contentLoadTexture(const char* path, HashID id)
{
	uint8_t* buffer;
	size_t size;
	ExternalAsset asset(path);
    auto texture = loadTexture(asset.buffer, asset.length);
    delete buffer;
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

void obliterateTexture(Texture* texture)
{
	glDeleteTextures(1, &texture->glName);
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
				((Font*)handles[i].item)->obliterate();
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
		((Font*)handle->item)->obliterate();
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
