/*
 * async_operations.cpp
 *
 *  Created on: 22 jul 2014
 *      Author: Lukas
 */

#include "async_operations.h"
#include "remote_debug.h"
#include "NDKHelper.h"

typedef struct {
	void* payload;
	asyncHandler handler;
	asyncDestructor dtor;
	const char* operationID;
} AsyncItem;

#define BUFFER_SIZE 128

static AsyncItem asyncBuffer[BUFFER_SIZE];
static size_t length;

void asyncOperation(void* ptr,
					asyncHandler handler,
					asyncDestructor dtor,
					const char* operationID) //operationID for debug
{
	//Run it right away to see if it finished instantly.
	auto res = handler(ptr);
	switch(res)
	{
		case ASYNC_OPERATION_COMPLETE:
			RLOGI("Async operation %s completed", operationID);
			break;
		case ASYNC_OPERATION_RUNNING:
			while(length == BUFFER_SIZE)
			{
				RLOGW("To many async operations in flight! %d", length);
				asyncOperationsProcess();
			}

			asyncBuffer[length++] = (AsyncItem){ptr, handler, dtor, operationID};
			break;
		case ASYNC_OPERATION_FAILURE:
			RLOGE("Async operation %s failed", operationID);
			break;
		default:
			RLOGE("Async operation %s returned invalid result %d", operationID, res);
			break;
	}
}

void asyncOperationsCancel()
{
	for (int i = 0; i < length; i++)
	{
		auto item = asyncBuffer[i];
		item.dtor(item.payload);
	}

	length = 0;
}

void asyncOperationsProcess()
{
	for(int i = 0; i < length; i++)
	{
		auto item = asyncBuffer[i];
		auto res  = item.handler(item.payload);
		switch(res)
		{
			case ASYNC_OPERATION_COMPLETE:
				asyncBuffer[i--] = asyncBuffer[--length];
				item.dtor(item.payload);

				RLOGI("Async operation %s completed", item.operationID);
				break;
			case ASYNC_OPERATION_RUNNING:
				//Do nothing
				break;
			case ASYNC_OPERATION_FAILURE:
				RLOGE("Async operation %s failed", item.operationID);
				asyncBuffer[i--] = asyncBuffer[--length];
				item.dtor(item.payload);
				break;
			default:
				RLOGE("Async operation %s returned invalid result %d", item.operationID, res);
				asyncBuffer[i--] = asyncBuffer[--length];
				item.dtor(item.payload);
				break;
		}
	}
}
