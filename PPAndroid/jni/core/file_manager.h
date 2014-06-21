/*
 * file_manager.h
 *
 *  Created on: Jun 19, 2014
 *      Author: Gustav
 */

#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#include "stdint.h"

extern "C" {

typedef struct
{
	uint32_t 	ip;
	uint16_t 	port;
	const char*	fileDirectory;
	bool	 	isDone;
} ReceiveFileConfig;

void receiveFiles(ReceiveFileConfig);
bool isDone(ReceiveFileConfig);
}

#endif /* FILE_MANAGER_H_ */
