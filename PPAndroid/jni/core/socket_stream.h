/*
 * socket_stream.h
 *
 *  Created on: Jun 19, 2014
 *      Author: Gustav
 */

#ifndef SOCKET_STREAM_H_
#define SOCKET_STREAM_H_

#include "buffer.h"
#include "stdint.h"

typedef struct {
	Buffer buffer;
	int socket;
	size_t size;
} SocketStream;

SocketStream* streamCreate(int socket, size_t bufferSize);
void streamDestroy(SocketStream* toDestroy);

uint8_t streamReadByte(SocketStream* stream);
uint16_t streamReadShort(SocketStream* stream);
uint32_t streamReadInt(SocketStream* stream);
uint64_t streamReadLong(SocketStream* stream);
float streamFloat(SocketStream* stream);
double streamDouble(SocketStream* stream);
size_t streamReadBytes(SocketStream* stream, uint8_t* dest, uint32_t length);
const uint8_t* streamReadInPlace(SocketStream* stream, uint32_t length);

#endif /* SOCKET_STREAM_H_ */
