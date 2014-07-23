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



typedef int (*asyncHandler)(void* context);
typedef void (*asyncDestructor)(void* context);

void asyncOperation(void* userPtr,
					asyncHandler funptr,
					asyncDestructor dtor,
					const char* operationID);

void asyncOperationsProcess();
void asyncOperationsCancel();


#endif /* ASYNC_OPERATIONS_H_ */
