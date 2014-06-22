/*
 * socket_stream.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Gustav
 */
#include "socket_stream.h"
#include <sys/socket.h>
#include "assert.h"
#include "buffer.h"

SocketStream* streamCreate(int socket, size_t bufferSize)
{
	SocketStream* stream = new SocketStream();
	stream->buffer.ptr = stream->buffer.base = new uint8_t[bufferSize];
	stream->buffer.length = 0;
	stream->socket = socket;
	stream->size = bufferSize;
	return stream;
}

void streamDestroy(SocketStream* toDestroy)
{
	LOGI("Destroying stream");
	delete [] toDestroy->buffer.base;
	LOGI("Destroyed buffer");
	delete toDestroy;
	LOGI("Destroyed stream");
}

static void streamReceive(SocketStream* stream)
{
	size_t length = bufferBytesRemaining(&stream->buffer);
	memmove(stream->buffer.base, stream->buffer.ptr, length);
	stream->buffer.ptr = stream->buffer.base;
	auto r = recv(stream->socket, stream->buffer.ptr + length, stream->size - length, 0);
	if (r < 0)
		LOGI("Error while receiving from stream: %d %s", errno, strerror(errno));
	stream->buffer.length = r + length;
}

uint8_t streamReadByte(SocketStream* stream)
{
	while(bufferBytesRemaining(&stream->buffer) < 1)
		streamReceive(stream);
    return bufferReadByte(&stream->buffer);
}

uint16_t streamReadShort(SocketStream* stream)
{
	while(bufferBytesRemaining(&stream->buffer) < 2)
		streamReceive(stream);
	return bufferReadShort(&stream->buffer);
}

uint32_t streamReadInt(SocketStream* stream)
{
	while(bufferBytesRemaining(&stream->buffer) < 4)
		streamReceive(stream);
	return bufferReadInt(&stream->buffer);
}

uint64_t streamReadLong(SocketStream* stream)
{
	while(bufferBytesRemaining(&stream->buffer) < 8)
		streamReceive(stream);
	return bufferReadLong(&stream->buffer);
}

float streamFloat(SocketStream* stream)
{
	while(bufferBytesRemaining(&stream->buffer) < 4)
		streamReceive(stream);
	return bufferReadFloat(&stream->buffer);
}

double streamDouble(SocketStream* stream)
{
	while(bufferBytesRemaining(&stream->buffer) < 8)
		streamReceive(stream);
	return bufferReadDouble(&stream->buffer);
}

size_t streamReadBytes(SocketStream* stream, uint8_t* dest, uint32_t length)
{
	ASSERT(length <= stream->size, "Stream buffer too small");
	while(bufferBytesRemaining(&stream->buffer) < length)
		streamReceive(stream);
	return bufferReadBytes(&stream->buffer, dest, length);
}

const uint8_t* streamReadInPlace(SocketStream* stream, uint32_t length)
{
	ASSERT(length <= stream->size, "Stream buffer too small");
	while(bufferBytesRemaining(&stream->buffer) < length)
		streamReceive(stream);
	auto result = stream->buffer.ptr;
	stream->buffer.ptr += length;
	return result;
}
