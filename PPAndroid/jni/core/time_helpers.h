/*
 * clock.h
 *
 *  Created on: Feb 21, 2014
 *      Author: Lukas_2
 */


#ifndef CLOCK_H_
#define CLOCK_H_


#include "stdint.h"

extern "C"
{
	typedef struct
	{
		uint64_t _totalTime, _lastTime, _elapsedTime;
		bool _isPaused, _isStopped;
	} Clock;

	float clockElapsed(Clock* clock);
	float clockTotal(Clock* clock);

	void clockStep(Clock* clock);
	void clockPause(Clock* clock);
	void clockResume(Clock* clock);
	void clockStart(Clock* clock);
	void clockStop(Clock* clock);
}

uint64_t timeNowMonoliticNsecs();
uint64_t timeTargetMonolitic(uint32_t msecs);

#endif
