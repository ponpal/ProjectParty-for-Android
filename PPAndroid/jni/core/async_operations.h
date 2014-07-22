/*
 * async_operations.h
 *
 *  Created on: 22 jul 2014
 *      Author: Lukas
 */

#ifndef ASYNC_OPERATIONS_H_
#define ASYNC_OPERATIONS_H_

enum
{
	ASYNC_OPERATION_COMPLETE = 0,
	ASYNC_OPERATION_RUNNING = 1,
	ASYNC_OPERATION_FAILURE = 2,
};



typedef int (*asyncHandler)(void* data);

void asyncOperation(void* userPtr, asyncHandler funptr, const char* operationID);
void asyncOperationsProcess();


#endif /* ASYNC_OPERATIONS_H_ */
