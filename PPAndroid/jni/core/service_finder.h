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
}


#endif /* SERVICEFINDER_H_ */
