#ifndef RESOURCE_TABLE
#define RESOURCE_TABLE

#include <string>
#include "asset_helper.h"
#include "assert.h"

#define NO_RESOURCE_ID "q"

template <class Resource>
class ResourceTable
{
	typedef void (*Obliterator)(Resource*);

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
	}

	~ResourceTable()
	{
		for(uint32_t i = 0; i < maxResources; i++)
		{
			if (resourcePaths[i] != NO_RESOURCE_ID)
				obliterator(&resources[i]);
			delete &resourcePaths[i];
		}
		delete resourcePaths;
		delete resources;
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

		obliterator(&resources[i]);
		resourcePaths[i] = NO_RESOURCE_ID;
		return true;
	}

	bool remove(uint32_t handle)
	{
		if (resourcePaths[handle] == NO_RESOURCE_ID)
			return false;
		obliterator(&resources[handle]);
		resourcePaths[handle] = NO_RESOURCE_ID;
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

		obliterator(&resources[i]);
		resources[i] = r;
		return i;

	}

	void removeAll()
	{
		for(uint32_t i = 0; i < maxResources; i++)
		{
			if (resourcePaths[i] != NO_RESOURCE_ID) {
				obliterator(&resources[i]);
                resourcePaths[i] = NO_RESOURCE_ID;
			}
		}
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

	const std::string& pathOf(uint32_t index)
	{
		ASSERTF(index < maxResources, "Trying to access resource out of bounds. Index: %d MaxIndex: %d", index, maxResources);
		return resourcePaths[index];
	}

	const Resource& operator[](uint32_t index)
	{
		ASSERTF(index < maxResources, "Trying to access resource out of bounds. Index: %d MaxIndex: %d", index, maxResources);
		return resources[index];
	}
};


#endif
