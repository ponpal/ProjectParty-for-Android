/*
 * buffer.cpp
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#include "buffer.h"
#include "assert.h"
#include "platform.h"

#define __STDC_FORMAT_MACROS

#include <inttypes.h>
#include "remote_debug.h"

#define BUFFER_WRITE_ERROR 1
#define BUFFER_READ_ERROR 2
#define BUFFER_UTF8_NOT_TERMINATED_ERROR 3

Buffer* bufferNew(uint32_t bufferSize)
{
	auto buffer = new Buffer();
	buffer->base = new uint8_t[bufferSize];
	buffer->ptr  = buffer->base;
	buffer->length = 0;
	buffer->capacity = bufferSize;
	buffer->error = 0;
	return buffer;
}

Buffer bufferWrapArray(uint8_t* array, uint32_t length)
{
	Buffer buffer;
	buffer.base     = buffer.ptr = array;
	buffer.length   = length;
	buffer.capacity = length;
	buffer.error    = 0;
	return buffer;
}

void bufferDelete(Buffer* buffer)
{
	RLOGI("Buffer delete %d", buffer);
	delete [] buffer->base;
	delete buffer;
}

bool bufferCheckError(Buffer* buffer)
{
	if(buffer == nullptr) return true;
	return buffer->error != 0;
}

uint32_t bufferBytesRemaining(Buffer* buffer)
{
	return buffer->length - (buffer->ptr - buffer->base);
}

uint32_t bufferBytesConsumed(Buffer* buffer)
{
	return buffer->ptr - buffer->base;
}

void bufferWriteUTF8(Buffer* buffer, const char* data)
{
	size_t length = strlen(data);
	if(!(bufferBytesRemaining(buffer) >= length))
	{
	    buffer->error = BUFFER_WRITE_ERROR;
	    return;
	}

	bufferWriteShort(buffer, length + 1);
	bufferWriteBytes(buffer, (uint8_t*)data, length + 1);
}

void bufferWriteBytes(Buffer* buffer, uint8_t* data, uint32_t length)
{
	if(!(bufferBytesRemaining(buffer) >= length))
	{
		buffer->error = BUFFER_WRITE_ERROR;
		return;
	}

	memcpy(buffer->ptr, data, length);
	buffer->ptr += length;
}

void bufferWriteByte(Buffer* buffer, uint8_t data)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(uint8_t)))
	{
		buffer->error = BUFFER_WRITE_ERROR;
		return;
	}

	*(buffer->ptr++) = data;
}

void bufferWriteShort(Buffer* buffer,uint16_t data)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(uint16_t)))
	{
		buffer->error = BUFFER_WRITE_ERROR;
		return;
	}

	auto ptr = buffer->ptr;
	*(ptr++) = (data & 0xFF);
	*(ptr++) = (data & 0xFF00) >> 8;
	buffer->ptr = ptr;
}

void bufferWriteInt(Buffer* buffer, uint32_t data)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(uint32_t)))
	{
		buffer->error = BUFFER_WRITE_ERROR;
		return;
	}

	auto ptr = buffer->ptr;
	*(ptr++) = (data & 0xFF);
	*(ptr++) = (data & 0xFF00) >> 8;
	*(ptr++) = (data & 0xFF0000) >> 16;
	*(ptr++) = (data & 0xFF000000) >> 24;
	buffer->ptr = ptr;
}

void bufferWriteLong(Buffer* buffer, uint64_t data)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(uint64_t)))
	{
		buffer->error = BUFFER_WRITE_ERROR;
		return;
	}

	auto ptr = buffer->ptr;
	*(ptr++) = ((uint64_t)data & 0xFF) >> 0;
	*(ptr++) = ((uint64_t)data & 0xFF00) >> 8;
	*(ptr++) = ((uint64_t)data & 0xFF0000) >> 16;
	*(ptr++) = ((uint64_t)data & 0xFF000000) >> 24;
	*(ptr++) = ((uint64_t)data & 0xFF00000000) >> 32;
	*(ptr++) = ((uint64_t)data & 0xFF0000000000) >> 40;
	*(ptr++) = ((uint64_t)data & 0xFF000000000000) >> 48;
	*(ptr++) = ((uint64_t)data & 0xFF00000000000000) >> 56;

	buffer->ptr = ptr;
}

void bufferWriteFloat(Buffer* buffer, float data)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(float)))
	{
		buffer->error = BUFFER_WRITE_ERROR;
		return;
	}


	union U
	{
		float a;
		uint32_t b;
	};

	U u;
	u.a = data;
	bufferWriteInt(buffer, u.b);
}

void bufferWriteDouble(Buffer* buffer, double data)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(double)))
	{
		buffer->error = BUFFER_WRITE_ERROR;
		return;
	}

	union U
	{
		double a;
		uint64_t b;
	};

	U u; u.a = data;
	bufferWriteLong(buffer, u.b);
}

char* bufferReadTempUTF8(Buffer* buffer)
{
	if(bufferBytesRemaining(buffer) < sizeof(uint16_t))
	{
		buffer->error = BUFFER_READ_ERROR;
		return nullptr;
	}

	size_t length = bufferReadShort(buffer);
	if(!(bufferBytesRemaining(buffer) >= sizeof(length)))
	{
		buffer->error = BUFFER_READ_ERROR;
		return nullptr;
	}

	char* ptr = (char*)buffer->ptr;
	buffer->ptr += length;

	if(*(buffer->ptr - 1) != '\0')
	{
		buffer->ptr = (uint8_t*)ptr;
		buffer->error = BUFFER_UTF8_NOT_TERMINATED_ERROR;
		return nullptr;
	}

	return ptr;
}

uint32_t bufferReadBytes(Buffer* buffer, uint8_t* dest, uint32_t numBytes)
{
	if(!(bufferBytesRemaining(buffer) >= numBytes))
	{
		buffer->error = BUFFER_READ_ERROR;
		return -1;
	}

	memcpy(dest, buffer->ptr, numBytes);
	buffer->ptr += numBytes;
	return numBytes;
}

uint8_t bufferReadByte(Buffer* buffer)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(uint8_t)))
	{
		buffer->error = BUFFER_READ_ERROR;
		return -1;
	}

	return *(buffer->ptr++);
}

uint16_t bufferReadShort(Buffer* buffer)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(uint16_t)))
	{
		buffer->error = BUFFER_READ_ERROR;
		return -1;
	}

	auto ptr = buffer->ptr;
	uint16_t result = *(ptr++);
	result = result | (*(ptr++) << 8);

	buffer->ptr = (uint8_t*)ptr;
	return result;
}

uint32_t bufferReadInt(Buffer* buffer)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(uint32_t)))
	{
		buffer->error = BUFFER_READ_ERROR;
		return -1;
	}

	auto ptr = buffer->ptr;
	uint32_t result = *(ptr++);
	result = result | (*(ptr++) << 8);
	result = result | (*(ptr++) << 16);
	result = result | (*(ptr++) << 24);
	buffer->ptr = ptr;
	return result;
}

uint64_t bufferReadLong(Buffer* buffer)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(uint64_t)))
	{
		buffer->error = BUFFER_READ_ERROR;
		return -1;
	}

	auto ptr = buffer->ptr;
	uint64_t result = bufferReadInt(buffer);
	result |= static_cast<uint64_t>(bufferReadInt(buffer)) << 32;
	return result;
}

float bufferReadFloat(Buffer* buffer)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(float)))
	{
		buffer->error = BUFFER_READ_ERROR;
		return -1;
	}

	union U { float a; uint32_t b; }; U u;

	u.b = bufferReadInt(buffer);
	return u.a;
}

double bufferReadDouble(Buffer* buffer)
{
	if(!(bufferBytesRemaining(buffer) >= sizeof(double)))
	{
		buffer->error = BUFFER_READ_ERROR;
		return -1;
	}

	union U { double  a; uint64_t b; }; U u;

	u.b = bufferReadLong(buffer);
	return u.a;
}
