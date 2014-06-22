/*
 * hash.h
 *
 *  Created on: Jun 18, 2014
 *      Author: Gustav
 */

#ifndef HASH_H_
#define HASH_H_
#include "stdint.h"

extern "C" {

typedef uint32_t HashID;
typedef uint16_t ShortHash;

HashID bytesHash(const void* buf, size_t len, size_t seed);
ShortHash shortHash(const void* buf, size_t len, size_t seed);

}

#endif /* HASH_H_ */
