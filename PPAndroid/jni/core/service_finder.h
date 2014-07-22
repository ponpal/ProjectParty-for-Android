/*
 * serviceFinder.h
 *
 *  Created on: 12 jul 2014
 *      Author: Lukas
 */

#ifndef SERVICEFINDER_H_
#define SERVICEFINDER_H_

#include "buffer.h"

extern "C"
{
	static const uint16_t servicePort = 34299;
	typedef void (*serviceFound)(const char*, Buffer*);
	struct ServiceFinder;

	ServiceFinder* serviceFinderCreate(const char* toFind, uint16_t port, serviceFound function);
	void serviceFinderDestroy(ServiceFinder*);
	void serviceFinderQuery(ServiceFinder*);
	bool serviceFinderPollFound(ServiceFinder*);

	typedef struct
	{
		const char* serviceID;
		Buffer* buffer;
		bool shouldContinue;
	} ServiceEvent;

	//The Context Pointer is a pointer that decides if the
	//ServiceFinder should continue to look for more stuff or not.
	typedef void (*foundService)(ServiceEvent*, bool success);
	void serviceFinderAsync(const char* toFind, uint16_t port,
							foundService handler, uint32_t interval);
}


#endif /* SERVICEFINDER_H_ */
