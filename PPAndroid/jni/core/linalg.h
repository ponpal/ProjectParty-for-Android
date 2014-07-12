/*
 * linalg.h
 *
 *  Created on: Jun 19, 2014
 *      Author: Gustav
 */

#ifndef LINALG_H_
#define LINALG_H_
#include "types.h"

extern "C"
{
	matrix4 matrixRotate(matrix4 toRotate, float angle);
	matrix4 matrixTranslate(matrix4 toTranslate, float x, float y);
	matrix4 matrixOrthogonalProjection(float left, float right, float bottom, float top);
}

#endif /* LINALG_H_ */
