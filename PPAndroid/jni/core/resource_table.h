#ifndef RESOURCE_TABLE
#define RESOURCE_TABLE

#include <string>
#include "asset_helper.h"
#include "assert.h"

#define NO_RESOURCE_ID "q"

template <class Resource>
class ResourceTable
{
	typedef void (*Obliterator)(Resource);

private:
	uint32_t maxResources;
	Resource*	resources;
	std::string* 			resourcePaths;
	Obliterator				obliterator;



public:

	ResourceTable(Obliterator o, uint32_t max)
		:obliterator(o), maxResources(max)
	{
		resources = new Resource[max];
		resourcePaths = new std::string[max];

		for (uint32_t i = 0; i < maxResources; i++)
		{
			resourcePaths[i] = NO_RESOURCE_ID;
		}

		for (auto& resource : resources)
		{
			resource = Resource();
		}
	}

	uint32_t add(const std::string& path, Resource resource)
	{
		uint32_t i = 0;
		for (; i < maxResources; i++)
		{
			if (resourcePaths[i] == NO_RESOURCE_ID)
				break;
		}

		resources[i] = resource;
		resourcePaths[i] = path;
		return i;
	}

	bool remove(const std::string& path)
	{
		uint32_t i = 0;
		for (; i < maxResources; i++)
		{
			if (resourcePaths[i] == path)
				break;
		}
		if (i == maxResources)
			return false;

		obliterator(resources[i]);
		resourcePaths[i] = NO_RESOURCE_ID;
		return true;
	}
	uint32_t replace(const std::string& path, Resource r)
	{
		uint32_t i = 0;
		for (; i < maxResources; i++)
		{
			if (resourcePaths[i] == path)
				break;
		}
		if (i == maxResources)
			return -1;

		obliterator(resources[i]);
		resources[i] = r;
		return i;

	}
	uint32_t indexOf(const std::string& path)
	{
		uint32_t i = 0;
		for (; i < maxResources; i++)
		{
			if (resourcePaths[i] == path)
				break;
		}
        return i != maxResources ? i : -1;
	}

	const Resource& operator[](uint32_t index)
	{
		assertf(index >= maxResources, "Trying to access resource out of bounds. Index: %d MaxIndex: %d", index, maxResources);
		return resources[index];
	}
};


#endif
