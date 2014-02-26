#ifndef RESOURCE_TABLE
#define RESOURCE_TABLE

#include <hash_map>
#include <string>
#include "asset_helper.h"
#include <tr1/unordered_map.h>

class ResourceTable
{
private:
	std::tr1::unordered_map<std::string, ExternalAsset*>* resources;

public:
	ResourceTable();
	void newExternal(std::string, const char*, size_t);
	void loadExternal(std::string);
	void unloadExternal(std::string);
	void reloadAll();
	void unloadAll();
};

#endif
