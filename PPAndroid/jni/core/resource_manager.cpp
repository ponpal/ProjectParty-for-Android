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

template<typename T> const HashID GetHash();

#define REGISTER_TYPE_HASH(type, name) \
    template<>const HashID GetHash<type>(){return bytesHash((name), strlen(name), 0);}

REGISTER_TYPE_HASH(Font, "Font");
REGISTER_TYPE_HASH(Frame, "Frame");

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

static void contentLoadFile(const char* path, uint8_t** buffer, size_t* size)
{
	LOGI("Trying to load file %s", path);
    auto file = fopen(path, "r+");
    ASSERTF(file != NULL, "Couldn't open file. %s", path);


    fseek( file, 0L, SEEK_END);
    size_t length = ftell(file);
    rewind(file);
    LOGI("ftell: %d", length);

    *buffer = new uint8_t[length];
    *size = length;

    fread(buffer, length, 1, file);
    fclose(file);
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
	contentLoadFile(path, &buffer, &size);
    auto texture = loadTexture(buffer, size);
    delete buffer;
    auto texPtr = new Frame(texture);
    return contentAddItem(id, GetHash<Frame>(), (void*) texPtr);
}

static std::string toString(uint32_t value)
{
	char buf[16];
	sprintf(buf, "%d", value);
	return std::string(buf);
}

Handle* contentLoad(const char* path)
{
	std::string hashPath = path::withoutExtension(path);
	HashID id = bytesHash(hashPath.c_str(), hashPath.size(), 0);
	std::string hashString = toString(id);
	std::string absPath = path::buildPath(resourcePath, hashString);

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

Handle* contentGetHandle(HashID id)
{
    for(int i = 0; i < handlesLength; i++)
	{
		if(handles[i].hashID == id)
			return &handles[i];
	}
    return nullptr;
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
				((Frame*)handles[i].item)->obliterate();
                delete (Frame*)handles[i].item;
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
	if(handle->typeID == GetHash<Frame>())
	{
		((Frame*)handle->item)->obliterate();
        delete (Frame*)handle->item;
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
