/*
 * buffer.cpp
 *
 *  Created on: Feb 22, 2014
 *      Author: Lukas_2
 */

#include "buffer.h"
#include "assert.h"

#define BOUNDS_CHECK(buffer, len) \
	assert(bufferBytesRemaining(buffer) >= len, "Buffer Overflow");


uint32_t bufferBytesRemaining(Buffer* buffer)
{
	return buffer->length - (buffer->ptr - buffer->base);
}

void bufferWriteUTF8(Buffer* buffer, const char* data)
{
	size_t length = strlen(data);
	BOUNDS_CHECK(buffer, length);

	bufferWriteShort(buffer, length);
	bufferWriteBytes(buffer, (uint8_t*)data, length);
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

	auto ptr = (uint16_t*)buffer->ptr;
	*(ptr++) = data;
	buffer->ptr = (uint8_t*)ptr;
}

void bufferWriteInt(Buffer* buffer, uint32_t data)
{
	BOUNDS_CHECK(buffer, sizeof(uint32_t));

	auto ptr = (uint32_t*)buffer->ptr;
	*(ptr++) = data;
	buffer->ptr = (uint8_t*)ptr;
}

void bufferWriteLong(Buffer* buffer, uint64_t data)
{
	BOUNDS_CHECK(buffer, sizeof(uint64_t));

	auto ptr = (uint64_t*)buffer->ptr;
	*(ptr++) = data;
	buffer->ptr = (uint8_t*)ptr;
}

void bufferWriteFloat(Buffer* buffer, float data)
{
	BOUNDS_CHECK(buffer, sizeof(float));

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
	BOUNDS_CHECK(buffer, sizeof(double));

	union U
	{
		double a;
		uint64_t b;
	};

	U u; u.a = data;
	bufferWriteLong(buffer, u.b);
}

void bufferReadUTF8(Buffer* buffer, const char* dest)
{
	size_t length = bufferReadShort(buffer);
	BOUNDS_CHECK(buffer, length);

	bufferReadBytes(buffer, (uint8_t*)dest, length);
}

void bufferReadBytes(Buffer* buffer, uint8_t* dest, size_t numBytes)
{
	BOUNDS_CHECK(buffer, numBytes);
	memcpy(dest, buffer->ptr, numBytes);
}

uint8_t bufferReadByte(Buffer* buffer)
{
	BOUNDS_CHECK(buffer, sizeof(uint8_t));
	return *(buffer->ptr++);
}

uint16_t bufferReadShort(Buffer* buffer)
{
	BOUNDS_CHECK(buffer, sizeof(uint16_t));

	auto ptr = (uint16_t*)buffer->ptr;
	auto result = *(ptr++);
	buffer->ptr = (uint8_t*)ptr;
	return result;
}

uint32_t bufferReadInt(Buffer* buffer)
{
	BOUNDS_CHECK(buffer, sizeof(uint32_t));

	auto ptr = (uint32_t*)buffer->ptr;
	auto result = *(ptr++);
	buffer->ptr = (uint8_t*)ptr;
	return result;
}

uint64_t bufferReadLong(Buffer* buffer)
{
	BOUNDS_CHECK(buffer, sizeof(uint64_t));

	auto ptr = (uint64_t*)buffer->ptr;
	auto result = *(ptr++);
	buffer->ptr = (uint8_t*)ptr;
	return result;
}

float bufferReadFloat(Buffer* buffer)
{
	BOUNDS_CHECK(buffer, sizeof(float));

	union U { float a; uint32_t b; };
	U u;

	auto ptr = (uint32_t*)buffer->ptr;
	u.b = *(ptr++);
	buffer->ptr = (uint8_t*)ptr;
	return u.a;
}

double bufferReadDouble(Buffer* buffer)
{
	BOUNDS_CHECK(buffer, sizeof(double));

	union U { double  a; uint64_t b; };
	U u;

	auto ptr = (uint32_t*)buffer->ptr;
	u.b = *(ptr++);
	buffer->ptr = (uint8_t*)ptr;
	return u.a;
}
