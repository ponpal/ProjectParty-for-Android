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

void contentInitialize(size_t numResources, std::string resourceFolder);
void contentTerminate();

bool contentIsPathLoaded(const char* path);
bool contentIsHashLoaded(HashID id);

Handle* contentLoad(const char* path);
Handle* contentGetHandle(HashID id);

bool contentUnloadPath(const char* path);
bool contentUnloadHandle(Handle* handle);

void contentUnloadAll();
}
#endif /* RESOURCE_MANAGER_H_ */
