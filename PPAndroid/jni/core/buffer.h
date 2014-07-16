/*
 * network.h
 *
 *  Created on: Feb 21, 2014
 *      Author: Lukas_2
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#include "stdint.h"

extern "C"
{
	typedef struct
	{
		uint8_t* base;
		uint8_t* ptr;
		uint32_t length;
		uint32_t capacity;
	} Buffer;

	Buffer* bufferCreate(size_t size);
	Buffer bufferWrapArray(uint8_t* array, size_t arraySize);
	void bufferDestroy(Buffer* buffer);

	uint32_t bufferBytesRemaining(Buffer* buffer);
	uint32_t bufferBytesConsumed(Buffer* buffer);

	//Output buffer
	void bufferWriteBytes(Buffer* buffer, uint8_t* data, size_t length);
	void bufferWriteByte(Buffer* buffer, uint8_t data);
	void bufferWriteShort(Buffer* buffer, uint16_t data);
	void bufferWriteInt(Buffer* buffer, uint32_t data);
	void bufferWriteLong(Buffer* buffer, uint64_t data);
	void bufferWriteUTF8(Buffer* buffer, const char* data);
	void bufferWriteFloat(Buffer* buffer, float data);
	void bufferWriteDouble(Buffer* buffer,double data);

	//Input Buffer
	size_t bufferReadBytes(Buffer* buffer, uint8_t* dest, size_t numBytes);
	char* bufferReadTempUTF8(Buffer* buffer);
	size_t bufferReadUTF8(Buffer* buffer, char** dest);
	uint8_t  bufferReadByte(Buffer* buffer);
	uint16_t bufferReadShort(Buffer* buffer);
	uint32_t bufferReadInt(Buffer* buffer);
	uint64_t bufferReadLong(Buffer* buffer);
	float  bufferReadFloat(Buffer* buffer);
	double bufferReadDouble(Buffer* buffer);
}

#endif /* NETWORK_H_ */
