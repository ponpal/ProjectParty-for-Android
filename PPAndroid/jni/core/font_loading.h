/*
 * font_loading.h
 *
 *  Created on: Feb 13, 2014
 *      Author: Lukas_2
 */

#ifndef FONT_LOADING_H_
#define FONT_LOADING_H_

#include "types.h"

Font constructFont(uint8_t* data, size_t dataSize, const Frame& page);

#endif /* FONT_LOADING_H_ */
