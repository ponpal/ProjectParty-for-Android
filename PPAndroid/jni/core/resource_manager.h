/*
 * resource_manager.h
 *
 *  Created on: Jun 18, 2014
 *      Author: Gustav
 */

#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_


#include "font.h"
#include "types.h"
#include <string>
#include <cstring>
#include "hash.h"

extern "C" {
typedef struct
{
	HashID hashID;
	HashID typeID;
	void* item;
}Handle;

typedef struct
{
	uint8_t* buffer;
	uint32_t length;
} Resource;

struct ResourceManager;

typedef struct ResourceManager
{
	const char* resourceDir;
	Resource (*loadResource)(struct ResourceManager* resources, const char* name);
    Handle* handles;
    size_t handlesLength;
} ResourceManager;

ResourceManager* resourceCreateLocal(size_t numResources);
ResourceManager* resourceCreateNetwork(size_t numResources, const char* resourceFolder);
void resourceTerminate(ResourceManager* resources);

bool resourceIsPathLoaded(ResourceManager* resources, const char* path);
bool resourceIsHashLoaded(ResourceManager* resources, HashID id);

Handle* resourceLoad(ResourceManager* resources, const char* path);
Handle* resourceGetHandle(ResourceManager* resources, HashID id);

bool resourceUnloadPath(ResourceManager* resources, const char* path);
bool resourceUnloadHandle(ResourceManager* resources, Handle* handle);

void resourceUnloadAll(ResourceManager* resources);
}
#endif /* RESOURCE_MANAGER_H_ */
