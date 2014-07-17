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
#include "remote_log.h"

#define BOUNDS_CHECK(buffer, len) \
if(!(bufferBytesRemaining(buffer) >= len)) \
{ \
	auto message___Buffer = new char[1024]; \
	sprintf(message___Buffer, "Buffer Overflow! Buf: %d, Len: %d, File: %s, Line: %d", \
		bufferBytesRemaining(buffer), len, __FILE__, __LINE__); \
	RLOGE("%s", message___Buffer); \
	delete [] message___Buffer; \
	platformExit(); \
}

Buffer* bufferCreate(size_t bufferSize)
{
	auto buffer = new Buffer();
	buffer->base = buffer->ptr = new uint8_t[bufferSize];
	buffer->length = 0;
	buffer->capacity = bufferSize;
	return buffer;
}

Buffer bufferWrapArray(uint8_t* array, size_t length)
{
	Buffer buffer;
	buffer.base     = buffer.ptr = array;
	buffer.length   = length;
	buffer.capacity = length;
	return buffer;
}

void bufferDestroy(Buffer* buffer)
{
	delete [] buffer->base;
	delete buffer;
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
	BOUNDS_CHECK(buffer, length);

	bufferWriteShort(buffer, length + 1);
	bufferWriteBytes(buffer, (uint8_t*)data, length + 1);
}

void bufferWriteBytes(Buffer* buffer, uint8_t* data, size_t length)
{
	BOUNDS_CHECK(buffer, length);

	memcpy(buffer->ptr, data, length);
	buffer->ptr += length;
}

void bufferWriteByte(Buffer* buffer, uint8_t data)
{
	BOUNDS_CHECK(buffer, sizeof(uint8_t));
	*(buffer->ptr++) = data;
}

void bufferWriteShort(Buffer* buffer,uint16_t data)
{
	BOUNDS_CHECK(buffer, sizeof(uint16_t));

	auto ptr = buffer->ptr;
	*(ptr++) = (data & 0xFF);
	*(ptr++) = (data & 0xFF00) >> 8;
	buffer->ptr = ptr;
}

void bufferWriteInt(Buffer* buffer, uint32_t data)
{
	BOUNDS_CHECK(buffer, sizeof(uint32_t));

	auto ptr = buffer->ptr;
	*(ptr++) = (data & 0xFF);
	*(ptr++) = (data & 0xFF00) >> 8;
	*(ptr++) = (data & 0xFF0000) >> 16;
	*(ptr++) = (data & 0xFF000000) >> 24;
	buffer->ptr = ptr;
}

void bufferWriteLong(Buffer* buffer, uint64_t data)
{
	BOUNDS_CHECK(buffer, sizeof(uint64_t));

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
	union U
	{
		double a;
		uint64_t b;
	};

	U u; u.a = data;
	bufferWriteLong(buffer, u.b);
}

size_t bufferReadUTF8(Buffer* buffer, char** dest)
{
	size_t length = bufferReadShort(buffer);
	BOUNDS_CHECK(buffer, length);
	*dest = new char[length];
	size_t size =  bufferReadBytes(buffer, (uint8_t*)(*dest), length);

	ASSERT(*(buffer->ptr - 1) == '\0', "Tried to read a string that was not null terminated!");
	return size;
}

char* bufferReadTempUTF8(Buffer* buffer)
{
	size_t length = bufferReadShort(buffer);
	BOUNDS_CHECK(buffer, length);
	char* ptr = (char*)buffer->ptr;
	buffer->ptr += length;

	ASSERT(*(buffer->ptr - 1) == '\0', "Tried to read a string that was not null terminated!");
	return ptr;
}

size_t bufferReadBytes(Buffer* buffer, uint8_t* dest, size_t numBytes)
{
	BOUNDS_CHECK(buffer, numBytes);
	memcpy(dest, buffer->ptr, numBytes);
	buffer->ptr += numBytes;
	return numBytes;
}

uint8_t bufferReadByte(Buffer* buffer)
{
	BOUNDS_CHECK(buffer, sizeof(uint8_t));
	return *(buffer->ptr++);
}

uint16_t bufferReadShort(Buffer* buffer)
{
	BOUNDS_CHECK(buffer, sizeof(uint16_t));

	auto ptr = buffer->ptr;
	uint16_t result = *(ptr++);
	result = result | (*(ptr++) << 8);

	buffer->ptr = (uint8_t*)ptr;
	return result;
}

uint32_t bufferReadInt(Buffer* buffer)
{
	BOUNDS_CHECK(buffer, sizeof(uint32_t));

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
	BOUNDS_CHECK(buffer, sizeof(uint64_t));

	auto ptr = buffer->ptr;
	uint64_t result = bufferReadInt(buffer);
	result |= static_cast<uint64_t>(bufferReadInt(buffer)) << 32;
	return result;
}

float bufferReadFloat(Buffer* buffer)
{
	BOUNDS_CHECK(buffer, sizeof(float));
	union U { float a; uint32_t b; }; U u;

	u.b = bufferReadInt(buffer);
	return u.a;
}

double bufferReadDouble(Buffer* buffer)
{
	BOUNDS_CHECK(buffer, sizeof(double));
	union U { double  a; uint64_t b; }; U u;

	u.b = bufferReadLong(buffer);
	return u.a;
}
