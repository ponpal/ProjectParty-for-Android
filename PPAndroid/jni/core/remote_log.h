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
	void remoteLogInitialize(const char* loggingID, uint16_t loggingPort);
	void remoteLuaLog(int verbosity, const char* message);
	void remoteLogFormat(int verbosity, const char* fmt, ...);
	void remoteLogTerm();
}


#define RLOGI(fmt, ...) remoteLogFormat(0, fmt, __VA_ARGS__)
#define RLOGW(fmt, ...) remoteLogFormat(0, fmt, __VA_ARGS__)
#define RLOGE(fmt, ...) remoteLogFormat(0, fmt, __VA_ARGS__)


#endif /* REMOTE_LOG_H_ */
