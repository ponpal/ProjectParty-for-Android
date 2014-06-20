/*
 * vibrator.h
 *
 *  Created on: Apr 22, 2014
 *      Author: Gustav
 */

#ifndef VIBRATOR_H_
#define VIBRATOR_H_

extern "C" {
int platformVibrate(uint64_t milliseconds);
uint16_t platformGetPort();
uint32_t platformGetIP();
uint32_t platformGetBroadcastAddress();
}
#endif /* VIBRATOR_H_ */
