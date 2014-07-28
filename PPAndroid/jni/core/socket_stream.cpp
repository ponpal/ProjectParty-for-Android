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

SocketStream* streamCreate(int socket, size_t bufferSize, int type)
{
	SocketStream* stream = new SocketStream();
	stream->buffer.ptr = stream->buffer.base = new uint8_t[bufferSize];
	if(type == INPUT_STREAM)
		stream->buffer.length = 0;
	else
		stream->buffer.length = bufferSize;
	stream->buffer.capacity = bufferSize;
	stream->socket = socket;

	stream->operationFailed = false;
	return stream;
}

uint32_t streamGetPosition(SocketStream* stream)
{
	return stream->buffer.ptr - stream->buffer.base;
}

void streamPosition(SocketStream* stream, uint32_t position)
{
	if(position < stream->buffer.capacity)
	{
		stream->buffer.ptr = stream->buffer.base + position;
		stream->operationFailed = false;
	}
	else
	{
		stream->operationFailed = true;
	}
}


Buffer* streamBuffer(SocketStream* stream)
{
	return &stream->buffer;
}

void streamDestroy(SocketStream* toDestroy)
{
	RLOGI("%s", "Destroying stream");
	delete [] toDestroy->buffer.base;
	delete toDestroy;
}

bool streamCheckError(SocketStream* stream)
{
	return stream->operationFailed != false;
}

uint32_t streamOutputDataLength(SocketStream* stream)
{
	auto buffer = stream->buffer;
	return buffer.ptr - buffer.base;
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
		RLOGE("%s %d %d", "Trying to send more data that can be sent in a single send",
			  length, stream->buffer.capacity);
		stream->operationFailed = true;
		return;
	}

	auto rem = bufferBytesRemaining(&stream->buffer);
	if(rem < length)
	{
		RLOGI("%s %d %d", "Need to flush buffer!", rem, length);
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


uint32_t streamInputDataLength(SocketStream* stream)
{
	return bufferBytesRemaining(&stream->buffer);
}

void streamReceive(SocketStream* stream)
{
	if(socketReceive(stream->socket, &stream->buffer))
	{
		stream->operationFailed = false;
	}
	else
	{
		stream->operationFailed = true;
	}
}

static bool receiveUntil(SocketStream* stream, uint32_t len)
{
	socketReceive(stream->socket, &stream->buffer);
	if(bufferBytesRemaining(&stream->buffer) >= len)
		return true;

	bool isBlocking = socketIsBlocking(stream->socket);
	if(isBlocking)
	{
		while(true)
		{
			auto size =  bufferBytesRemaining(&stream->buffer);
			if(size >= len) break;

			if(!socketReceive(stream->socket, &stream->buffer))
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
	RLOGI("READ LONG %d, %d", stream, stream->buffer);

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
	ASSERT(length <= stream->buffer.capacity, "Stream buffer too small");
	if(bufferBytesRemaining(&stream->buffer) < length)
	{
		auto success = receiveUntil(stream, length);
		if(!success) return -1;
	}

	return bufferReadBytes(&stream->buffer, dest, length);
}


const char* streamReadTempUTF8(SocketStream* stream)
{
	auto len = streamReadShort(stream);
	ASSERT(len <= stream->buffer.capacity, "Stream buffer to small!");
	stream->buffer.ptr -= 2; //Wierd but ok.
	return bufferReadTempUTF8(&stream->buffer);
}

const uint8_t* streamReadInPlace(SocketStream* stream, uint32_t length)
{
	ASSERT(length <= stream->buffer.capacity, "Stream buffer too small");
	if(bufferBytesRemaining(&stream->buffer) < length)
	{
		auto success = receiveUntil(stream, length);
		if(!success) return (const uint8_t*)0;
	}

	auto result = stream->buffer.ptr;
	stream->buffer.ptr += length;
	return result;
}
