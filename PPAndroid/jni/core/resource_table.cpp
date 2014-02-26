/*
 * resource_table.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gustav
 */

#include "resource_table.h"
#include "asset_helper.h"


ResourceTable::ResourceTable()
{
	this->resources = new std::tr1::unordered_map();
}
void ResourceTable::newExternal(std::string filePath, const char* buffer, uint32_t size)
{
	ExternalAsset* asset = new ExternalAsset(filePath, buffer, size);
	ResourceTable::resources->insert(filePath, asset);
}
void ResourceTable::loadExternal(std::string filePath)
{
	ExternalAsset* asset = new ExternalAsset(filePath);
	ResourceTable::resources->insert(filePath, asset);
}
void ResourceTable::unloadExternal(std::string filePath)
{
	delete this->resources[filePath];
}
void ResourceTable::reloadAll()
{
	for(
        unordered_map<std::string, ExternalAsset>::iterator it = c1.begin();
		it != c1.end();
		++it)
	{
		this->resources[it->first] = new ExternalAsset(filePath);
	}
}
void ResourceTable::unloadAll()
{
	for(
        unordered_map<std::string, ExternalAsset>::iterator it = c1.begin();
		it != c1.end();
		++it)
	{
		delete this->resources[it->first];
	}
}
