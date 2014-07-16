/*
 * file_manager.h
 *
 *  Created on: Jun 19, 2014
 *      Author: Gustav
 */

#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#include "stdint.h"
#include "socket_stream.h"

extern "C" {

enum {
	TASK_SUCCESS = 0,
	TASK_FAILURE = -1,
	TASK_PROCESSING = 1
};


bool receiveFile(SocketStream* stream, const char* fileDirectory, const char* name);
uint32_t receiveFiles(uint32_t ip, uint16_t port, const char* fileDirectory);
int32_t receiveFilesStatus(uint32_t);

}

#endif /* FILE_MANAGER_H_ */
