/*
 * sensor.cpp
 *
 *  Created on: 4 mar 2014
 *      Author: Pontus
 */

#include "sensor.h"

GESTURE_STATE tapDetect(TapDetector* detector, const AInputEvent* motion_event)
{
	if( AMotionEvent_getPointerCount( motion_event ) > 1 )
	{
		//Only support single touch
		return false;
	}

	int32_t action = AMotionEvent_getAction( motion_event );
	unsigned int flags = action & AMOTION_EVENT_ACTION_MASK;
	switch( flags )
	{
		case AMOTION_EVENT_ACTION_DOWN:
			detector->pointerID = AMotionEvent_getPointerId( motion_event, 0 );
			detector->x = AMotionEvent_getX( motion_event, 0 );
			detector->y = AMotionEvent_getY( motion_event, 0 );
			break;
		case AMOTION_EVENT_ACTION_UP:
		{
			int64_t eventTime = AMotionEvent_getEventTime( motion_event );
			int64_t downTime = AMotionEvent_getDownTime( motion_event );
			if( eventTime - downTime <= TAP_TIMEOUT )
			{
				if( detector->pointerID == AMotionEvent_getPointerId( motion_event, 0 ) )
				{
					float x = AMotionEvent_getX( motion_event, 0 ) - detector->x;
					float y = AMotionEvent_getY( motion_event, 0 ) - detector->y;
					if( x * x + y * y < TOUCH_SLOP * TOUCH_SLOP)
					{
						return 5; //my gesture state action here.
					}
				}
			}
			break;
		}
	}
	return 0; //my gesture state none here
}
