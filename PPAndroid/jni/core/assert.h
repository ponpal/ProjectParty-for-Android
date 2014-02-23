/*
 * assert.h
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#ifndef ASSERT_H_
#define ASSERT_H_

#ifdef NO_ASSERTS
#define assertf(b, fmt, msg)
#define assert(b, msg)

#else
#include <cstdlib>
#include "NDKHelper.h"

#define assert(b, msg) \
if(!(b)) \
{ \
	LOGI("Assertion Failure: File: %s Line: %d Msg: %s", __FILE__, __LINE__, msg); \
	exit(-1); \
}
#define assertf(b, fmt, ...) \
if(!(b)) \
{ \
	LOGI("Assertion Failure: File: %s Line: %d \n" fmt, __FILE__, __LINE__, __VA_ARGS__); \
	exit(-1); \
}

#endif
#endif /* ASSERT_H_ */
