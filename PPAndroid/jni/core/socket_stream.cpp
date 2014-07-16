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
#include "socket_helpers.h"

SocketStream* streamCreate(int socket, size_t bufferSize)
{
	SocketStream* stream = new SocketStream();
	stream->buffer.ptr = stream->buffer.base = new uint8_t[bufferSize];
	stream->buffer.length = 0;
	stream->socket = socket;
	stream->size = bufferSize;
	stream->operationFailed = false;
	return stream;
}

void streamDestroy(SocketStream* toDestroy)
{
	RLOGI("%s", "Destroying stream");
	delete [] toDestroy->buffer.base;
	delete toDestroy;
}


bool streamHasOutputData(SocketStream* stream)
{
	auto buffer = stream->buffer;
	return buffer.ptr != buffer.base;
}

void streamFlush(SocketStream* stream, bool untilFinished)
{
	bool isBlocking = socketIsBlocking(stream->socket);
	if(!isBlocking && untilFinished)
	{
		socketSetBlocking(stream->socket, true);
	}

	auto success = socketTCPSend(stream->socket, &stream->buffer);
	if(!success)
	{
		RLOGE("%s", "Failed to flush stream!");
		stream->operationFailed = true;


		if(!isBlocking && untilFinished)
		{
			socketSetBlocking(stream->socket, false);
		}
		return;
	}

	if(!isBlocking && untilFinished)
	{
		socketSetBlocking(stream->socket, false);
	}

	stream->operationFailed = false;
}

void streamWriteByte(SocketStream* stream, uint8_t data)
{
	auto rem = bufferBytesRemaining(&stream->buffer);
	if(rem < 1)
	{
		streamFlush(stream, true);
		if(stream->operationFailed)
			return;
	}

	bufferWriteByte(&stream->buffer, data);
	stream->operationFailed = false;
}

void streamWriteShort(SocketStream* stream, uint16_t data)
{
	auto rem = bufferBytesRemaining(&stream->buffer);
	if(rem < 2)
	{
		streamFlush(stream, true);
		if(stream->operationFailed)
			return;
	}

	bufferWriteShort(&stream->buffer, data);
	stream->operationFailed = false;
}

void streamWriteInt(SocketStream* stream, uint32_t data)
{
	auto rem = bufferBytesRemaining(&stream->buffer);
	if(rem < 4)
	{
		streamFlush(stream, true);
		if(stream->operationFailed)
			return;
	}

	bufferWriteInt(&stream->buffer, data);
	stream->operationFailed = false;
}

void streamWriteLong(SocketStream* stream, uint64_t data)
{
	auto rem = bufferBytesRemaining(&stream->buffer);
	if(rem < 8)
	{
		streamFlush(stream, true);
		if(stream->operationFailed)
			return;
	}

	bufferWriteLong(&stream->buffer, data);
	stream->operationFailed = false;
}

void streamWriteFloat(SocketStream* stream, float data)
{
	auto rem = bufferBytesRemaining(&stream->buffer);
	if(rem < 4)
	{
		streamFlush(stream, true);
		if(stream->operationFailed)
			return;
	}

	bufferWriteFloat(&stream->buffer, data);
	stream->operationFailed = false;
}

void streamWriteDouble(SocketStream* stream, double data)
{
	auto rem = bufferBytesRemaining(&stream->buffer);
	if(rem < 8)
	{
		streamFlush(stream, true);
		if(stream->operationFailed)
			return;
	}

	bufferWriteDouble(&stream->buffer, data);
	stream->operationFailed = false;
}

void streamWriteBytes(SocketStream* stream, uint8_t* data, uint32_t length)
{
	if(length > stream->buffer.capacity)
	{
		RLOGE("%s", "Trying to send more data that can be sent in a single send");
		stream->operationFailed = true;
		return;
	}

	auto rem = bufferBytesRemaining(&stream->buffer);
	if(rem < length)
	{
		streamFlush(stream, true);
		if(stream->operationFailed)
			return;
	}

	bufferWriteBytes(&stream->buffer, data, length);
	stream->operationFailed = false;
}


void streamWriteUTF8(SocketStream* stream, const char* data)
{
	auto len = strlen(data) + 1;
	streamWriteBytes(stream, (uint8_t*)data, len);
}


static bool streamReceive(SocketStream* stream)
{
	size_t length = bufferBytesRemaining(&stream->buffer);
	memmove(stream->buffer.base, stream->buffer.ptr, length);
	stream->buffer.ptr = stream->buffer.base;
	auto r = recv(stream->socket, stream->buffer.ptr + length, stream->size - length, 0);
	if (r < 0)
	{
		RLOGI("Error while receiving from stream: %d %s", errno, strerror(errno));
		return false;
	}

	stream->buffer.length = r + length;
	return true;
}

bool streamHasInputData(SocketStream* stream)
{
	auto buffer = stream->buffer;
	bool hasData = buffer.ptr - buffer.base < buffer.length;
	if(!hasData)
	{
		streamReceive(stream);
		hasData = buffer.ptr - buffer.base < buffer.length;
	}

	return hasData;
}

static bool receiveUntil(SocketStream* stream, uint32_t len)
{
	streamReceive(stream);
	if(bufferBytesRemaining(&stream->buffer) >= len)
		return true;

	bool isBlocking = socketIsBlocking(stream->socket);
	if(isBlocking)
	{
		while(true)
		{
			auto size =  bufferBytesRemaining(&stream->buffer);
			if(size >= len) break;

			if(!streamReceive(stream))
			{
				RLOGE("%s", "Failed to receive data!");
				stream->operationFailed = true;
				return false;
			}
		}
	}
	else
	{
		RLOGW("%s", "Failed to receive data!");
		stream->operationFailed = true;
		return false;
	}

	stream->operationFailed = false;
	return true;
}


uint8_t streamReadByte(SocketStream* stream)
{
	if(bufferBytesRemaining(&stream->buffer) < 1)
	{
		auto success = receiveUntil(stream, 1);
		if(!success) return -1;
	}

	return bufferReadByte(&stream->buffer);
}

uint16_t streamReadShort(SocketStream* stream)
{
	if(bufferBytesRemaining(&stream->buffer) < 2)
	{
		auto success = receiveUntil(stream, 2);
		if(!success) return -1;
	}

	return bufferReadShort(&stream->buffer);
}

uint32_t streamReadInt(SocketStream* stream)
{
	if(bufferBytesRemaining(&stream->buffer) < 4)
	{
		auto success = receiveUntil(stream, 4);
		if(!success) return -1;
	}

	return bufferReadInt(&stream->buffer);
}

uint64_t streamReadLong(SocketStream* stream)
{
	if(bufferBytesRemaining(&stream->buffer) < 8)
	{
		auto success = receiveUntil(stream, 8);
		if(!success) return -1;
	}

	return bufferReadLong(&stream->buffer);
}

float streamFloat(SocketStream* stream)
{
	if(bufferBytesRemaining(&stream->buffer) < 4)
	{
		auto success = receiveUntil(stream, 4);
		if(!success) return -1;
	}

	return bufferReadFloat(&stream->buffer);
}

double streamDouble(SocketStream* stream)
{
	if(bufferBytesRemaining(&stream->buffer) < 8)
	{
		auto success = receiveUntil(stream, 8);
		if(!success) return -1;
	}

	return bufferReadDouble(&stream->buffer);
}

size_t streamReadBytes(SocketStream* stream, uint8_t* dest, uint32_t length)
{
	ASSERT(length <= stream->size, "Stream buffer too small");
	if(bufferBytesRemaining(&stream->buffer) < length)
	{
		auto success = receiveUntil(stream, length);
		if(!success) return -1;
	}

	return bufferReadBytes(&stream->buffer, dest, length);
}

const uint8_t* streamReadInPlace(SocketStream* stream, uint32_t length)
{
	ASSERT(length <= stream->size, "Stream buffer too small");
	if(bufferBytesRemaining(&stream->buffer) < length)
	{
		auto success = receiveUntil(stream, length);
		if(!success) return (const uint8_t*)0;
	}

	auto result = stream->buffer.ptr;
	stream->buffer.ptr += length;
	return result;
}
