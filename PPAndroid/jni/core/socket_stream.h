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

extern "C"
{
	typedef struct {
		Buffer buffer;
		int socket;
		bool operationFailed;
	} SocketStream;

	enum
	{
		INPUT_STREAM = 0,
		OUTPUT_STREAM = 1
	};

	SocketStream* streamCreate(int socket, size_t bufferSize, int type);
	void streamDestroy(SocketStream* toDestroy);
	void streamReceive(SocketStream* stream);
	void streamFlush(SocketStream* stream, bool untilFinished);

	uint32_t streamGetPosition(SocketStream* stream);
	void streamPosition(SocketStream* stream, uint32_t position);

	uint32_t streamInputDataLength(SocketStream* stream);
	uint32_t streamOutputDataLength(SocketStream* stream);
	Buffer* streamBuffer(SocketStream* stream);
	bool streamCheckError(SocketStream* stream);

	//Input
	uint8_t streamReadByte(SocketStream* stream);
	uint16_t streamReadShort(SocketStream* stream);
	uint32_t streamReadInt(SocketStream* stream);
	uint64_t streamReadLong(SocketStream* stream);
	float streamFloat(SocketStream* stream);
	double streamDouble(SocketStream* stream);
	size_t streamReadBytes(SocketStream* stream, uint8_t* dest, uint32_t length);
	const char* streamReadTempUTF8(SocketStream* stream);
	const uint8_t* streamReadInPlace(SocketStream* stream, uint32_t length);

	//Output
	void streamWriteByte(SocketStream* stream, uint8_t data);
	void streamWriteShort(SocketStream* stream, uint16_t data);
	void streamWriteInt(SocketStream* stream, uint32_t data);
	void streamWriteLong(SocketStream* stream, uint64_t data);
	void streamWriteFloat(SocketStream* stream, float data);
	void streamWriteDouble(SocketStream* stream, double data);
	void streamWriteBytes(SocketStream* stream, uint8_t* data, uint32_t length);
	void streamWriteUTF8(SocketStream* stream, const char* data);
}

#endif /* SOCKET_STREAM_H_ */
