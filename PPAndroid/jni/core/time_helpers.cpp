/*
 * clock.cpp
 *
 *  Created on: Feb 21, 2014
 *      Author: Lukas_2
 */
#include "time_helpers.h"
#include <time.h>

uint64_t toNsecs(timespec spec)
{
	uint64_t result = spec.tv_sec;
	result *= 1000000000;
	result += spec.tv_nsec;
	return result;
}

uint64_t getCurrentTime()
{
	timespec spec;
	clock_gettime(CLOCK_MONOTONIC, &spec);
	return toNsecs(spec);
}

uint64_t timeNowMonoliticNsecs()
{
	return getCurrentTime();
}

uint64_t timeTargetMonolitic(uint32_t msecs)
{
	uint64_t time = msecs;
	time *= 1000000;
	time += timeNowMonoliticNsecs();
	return time;
}

///Should only be used for < second time messurement.
uint32_t timeRelativeClock(Clock* clock)
{
	uint64_t now = timeNowMonoliticNsecs();
	uint64_t elapsed = now - clock->_lastTime;
	return (uint32_t)elapsed;
}

void clockStart(Clock* clock)
{
	clock->_totalTime = 0;
	clock->_elapsedTime = 0;
	clock->_lastTime = getCurrentTime();
	clock->_isPaused = false;
	clock->_isStopped = false;
}

void clockStop(Clock* clock)
{
	clock->_elapsedTime = 0;
	clock->_isStopped = true;
}

void clockStep(Clock* clock)
{
	if(clock->_isPaused ||
	   clock->_isStopped) return;

	uint64_t tmp = getCurrentTime();
	clock->_elapsedTime = tmp - clock->_lastTime;
	clock->_totalTime  += clock->_elapsedTime;
	clock->_lastTime   = tmp;
}

void clockPause(Clock* clock)
{
	clockStep(clock);
	clock->_isPaused = true;
}

void clockResume(Clock* clock)
{
	clock->_lastTime = getCurrentTime();
	clock->_isPaused = false;
}

float clockTotal(Clock* clock)
{
	return clock->_totalTime / 1e9;
}

float clockElapsed(Clock* clock)
{
	return clock->_elapsedTime / 1e9;
}
