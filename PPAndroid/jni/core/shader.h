/*
 * shader.h
 *
 *  Created on: Feb 12, 2014
 *      Author: Lukas_2
 */

#ifndef SHADER_H_
#define SHADER_H_

#include <GLES2/gl2.h>

namespace shader
{
	GLuint loadShader(const char* source);
	GLuint createProgram(const char* vertexSource, const char* fragmentSource);
}

#endif /* SHADER_H_ */
