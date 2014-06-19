/*
 * linalg.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Gustav
 */

#include "types.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "linalg.h"

matrix4 matrixRotate(matrix4 toRotate, float angle)
{
	glm::mat4 mat = *((glm::mat4*)&toRotate);
	mat = glm::rotate(mat, angle, glm::vec3(0,0,1));
	return *((matrix4*)&mat);
}

matrix4 matrixTranslate(matrix4 toTranslate, float x, float y)
{
	glm::mat4 mat = *((glm::mat4*)&toTranslate);
	mat = glm::translate(mat, glm::vec3(x, y, 0));
	return *((matrix4*)&mat);
}

matrix4 matrixOrthogonalProjection(float left, float right, float bottom, float top)
{
	glm::mat4 mat = glm::ortho(left, right, bottom, top);
	return *((matrix4*)&mat);
}


