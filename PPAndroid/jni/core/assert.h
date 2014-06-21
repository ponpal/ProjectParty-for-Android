/*
 * assert.h
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#ifndef ASSERT_H_
#define ASSERT_H_

#ifdef NO_ASSERTS
#define ASSERT(b, msg)
#define ASSERTF(b, fmt, msg)
#else
#include <cstdlib>
#include "JNIHelper.h"
#include "game.h"
#include "network.h"

#define ASSERT(b, msg) \
if(!(b)) \
{ \
	auto message___Buffer = new char[1024]; \
	sprintf(message___Buffer, "Assertion Failure: File: %s Line: %d Msg: %s", __FILE__, __LINE__, msg); \
	LOGE("%s", message___Buffer); \
	delete message___Buffer; \
}
#define ASSERTF(b, fmt, ...) \
if(!(b)) \
{ \
	auto message___Buffer = new char[1024]; \
	sprintf(message___Buffer, "Assertion Failure: File: %s Line: %d \n" fmt, __FILE__, __LINE__, __VA_ARGS__); \
	LOGE("%s", message___Buffer); \
	delete message___Buffer; \
}

#endif
#endif /* ASSERT_H_ */
