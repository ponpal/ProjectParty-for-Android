/*
 * remote_log.h
 *
 *  Created on: 10 jul 2014
 *      Author: Lukas
 */

#ifndef REMOTE_LOG_H_
#define REMOTE_LOG_H_

#include <cstdlib>
extern "C"
{
	void remoteDebugInitialize();
	void remoteDebugStart(const char* debugID);
	void remoteDebugStop();

	void remoteLog(int verbosity, const char* message);
	void remoteLogFormat(int verbosity, const char* fmt, ...);
	void remoteDebugUpdate();
}


#define RLOGI(fmt, ...) remoteLogFormat(0, fmt, __VA_ARGS__)
#define RLOGW(fmt, ...) remoteLogFormat(1, fmt, __VA_ARGS__)
#define RLOGE(fmt, ...) remoteLogFormat(2, fmt, __VA_ARGS__)


#endif /* REMOTE_LOG_H_ */
