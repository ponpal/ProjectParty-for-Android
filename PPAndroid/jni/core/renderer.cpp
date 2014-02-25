/*
 * renderer.cpp
 *
 *  Created on: Feb 12, 2014
// *      Author: Lukas_2
// */
//
//#include "renderer.h"
//#include "types.h"
//#include "shader.h"
//#include <GLES2/gl2.h>
//#include "log.h"
//#include <cstdlib>
//#include <cmath>
//#include "font.h"
//#include "external_libs/utf8.h"
//
//static const char gVertexShader[] =
//    "attribute vec2 vPosition;\n"
//	"attribute vec4 vColor;\n"
//	"varying vec4 color;\n"
//	"attribute vec2 vTexcoord;\n"
//	"varying vec2 texcoord;\n"
//	"uniform mat4 transform;"
//	"void main() {\n"
//	"  color       = vColor;\n"
//	"  texcoord    = vTexcoord;\n"
//    "  gl_Position = transform * vec4(vPosition,0,1);\n"
//    "}\n";
//
//static const char gFragmentShader[] =
//    "precision mediump float;\n"
//	"uniform sampler2D sampler;\n"
//	"varying vec2 texcoord;\n"
//	"varying vec4 color;\n"
//    "void main() {\n"
//    "  gl_FragColor = texture2D(sampler, texcoord) * color;\n"
//    "}\n";
//
//
//void checkGlError(const char* op) {
//    for (GLint error = glGetError(); error; error
//            = glGetError()) {
//        LOGI("after %s() glError (0x%x)\n", op, error);
//    }
//}
//
//GLuint createVBO(size_t count, size_t elementSize)
//{
//    GLuint buffer;
//    glGenBuffers(1, &buffer);
//    glBindBuffer(GL_ARRAY_BUFFER, buffer);
//    glBufferData(GL_ARRAY_BUFFER, count * elementSize, NULL, GL_STREAM_DRAW);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//    LOGI("Buffer Size: %d", count * elementSize);
//
//    checkGlError("createVBO");
//    return buffer;
//}
//
//GLuint createIBO(size_t count)
//{
//	GLuint buffer;
//	glGenBuffers(1, &buffer);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
//
//	std::vector<GLushort> indices;
//	indices.reserve(count * 6);
//	for(GLushort i = 0; i < count * 4; i += 4) {
//		indices.push_back(i);
//		indices.push_back(i + 1);
//		indices.push_back(i + 2);
//
//		indices.push_back(i + 1);
//		indices.push_back(i + 2);
//		indices.push_back(i + 3);
//	}
//
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * 6 * sizeof(short), &indices[0], GL_STATIC_DRAW);
//
//	checkGlError("createIBO");
//	return buffer;
//}
//
//
//
//Renderer::Renderer(size_t maxBatchSize)
//{
//	size_t actualSize = maxBatchSize * 4;
//
//	vertices = new Vertex[actualSize];
//	elements = 0;
//	capacity = actualSize;
//
//
//	vbo = createVBO(actualSize, sizeof(Vertex));
//	ibo = createIBO(maxBatchSize);
//
//	program = shader::createProgram(gVertexShader, gFragmentShader);
//	samplerUniform 		= glGetUniformLocation(program, "sampler");
//	transformUniform 	= glGetUniformLocation(program, "transform");
//
//	posAttrib   	= glGetAttribLocation(program, "vPosition");
//	colorAttrib 	= glGetAttribLocation(program, "vColor");
//	texcoordAttrib 	= glGetAttribLocation(program, "vTexcoord");
//}
//
//Renderer::~Renderer()
//{
//	glDeleteProgram(program);
//	glDeleteBuffers(1, &vbo);
//	glDeleteBuffers(1, &ibo);
//}
//
//
//void Renderer::setTransform(const glm::mat4& matrix)
//{
//	transform = matrix;
//}
//
//
//void Renderer::flushIfNeeded(Texture texture, size_t count)
//{
//	if( texture.glName != activeTexture.glName || elements + count > capacity)
//	{
//		draw();
//		activeTexture = texture;
//	}
//}
//
//
//void Renderer::addFrame(const Frame& frame,
//						glm::vec2 bl,
//						glm::vec2 dim,
//						Color color)
//{
//	flushIfNeeded(frame.texture, 4);
//	auto ptr = &vertices[elements];
//
//	glm::vec2 bottomLeft = glm::vec2(frame.coords.x, frame.coords.y);
//	glm::vec2 topRight   = glm::vec2(frame.coords.z, frame.coords.w);
//
//
//	Vertex vert;
//	vert.position = bl;
//	vert.texcoord = bottomLeft;
//	vert.color    = color;
//
//	*(ptr++) = vert;
//	vert.position = bl + glm::vec2(dim.x, 0);
//	vert.texcoord = bottomLeft + glm::vec2(topRight.x, 0);
//	*(ptr++) = vert;
//
//	vert.position = bl + glm::vec2(0, dim.y);
//	vert.texcoord = bottomLeft + glm::vec2(0, topRight.y);
//	*(ptr++) = vert;
//
//	vert.position = bl + dim;
//	vert.texcoord = bottomLeft + topRight;
//	*(ptr++) = vert;
//
//	elements += 4;
//}
//
//inline glm::vec2 rotated(glm::vec2 pos, glm::vec2 offset, float sinus, float cosinus)
//{
//	pos.x += offset.x * cosinus - offset.y * sinus;
//	pos.y += offset.x * sinus   + offset.y * cosinus;
//	return pos;
//}
//
//void Renderer::addFrame(const Frame& frame,
//						glm::vec2 pos,
//						glm::vec2 dim,
//						Color color,
//						glm::vec2 origin,
//						float rotation,
//						bool mirrored)
//{
//	flushIfNeeded(frame.texture, 4);
//
//	glm::vec2 botLeft  = glm::vec2(frame.coords.x, frame.coords.y);
//	glm::vec2 topRight = glm::vec2(frame.coords.z + frame.coords.x, frame.coords.w + frame.coords.y);
//
//	if(mirrored)
//	{
//		float tmp = botLeft.x;
//		botLeft.x = topRight.x;
//		topRight.x = tmp;
//	}
//
//	Vertex vert;
//	vert.color    = color;
//
//	float sinus = sin(rotation),
//		cosinus = cos(rotation);
//
//	auto ptr = &vertices[elements];
//
//	vert.position = rotated(pos, -origin, sinus, cosinus);
//	vert.texcoord = botLeft;
//	*(ptr++) = vert;
//
//	vert.position = rotated(pos, -origin + glm::vec2(0, dim.y), sinus, cosinus);
//	vert.texcoord = glm::vec2(botLeft.x, topRight.y);
//	*(ptr++) = vert;
//
//	vert.position = rotated(pos, -origin + glm::vec2(dim.x, 0), sinus, cosinus);
//	vert.texcoord = glm::vec2(topRight.x, botLeft.y);
//	*(ptr++) = vert;
//
//	vert.position = rotated(pos, -origin + dim, sinus, cosinus);
//	vert.texcoord = topRight;
//	*(ptr++) = vert;
//
//	elements += 4;
//}
//
//void Renderer::drawChar(Texture texture, const CharInfo& info, glm::vec2 pos, glm::vec2 offset, Color color)
//{
//	flushIfNeeded(texture, 4);
//	auto ptr = &vertices[elements];
//
//
//	glm::vec2 location   = pos + offset;
//	glm::vec2 bottomLeft = glm::vec2(info.textureCoords.x, info.textureCoords.y);
//	glm::vec2 topRight   = glm::vec2(info.textureCoords.z, info.textureCoords.w);
//	glm::vec2 dim        = glm::vec2(info.srcRect.z, info.srcRect.w);
//
//	Vertex vert;
//	vert.position = location;
//	vert.texcoord = bottomLeft;
//	vert.color    = color;
//	*(ptr++) = vert;
//
//	vert.position = location + glm::vec2(dim.x, 0);
//	vert.texcoord = glm::vec2(topRight.x, bottomLeft.y);
//	*(ptr++) = vert;
//
//	vert.position = location + glm::vec2(0, dim.y);
//	vert.texcoord = glm::vec2(bottomLeft.x, topRight.y);;
//	*(ptr++) = vert;
//
//	vert.position = location + dim;
//	vert.texcoord = topRight;
//	*(ptr++) = vert;
//
//	elements += 4;
//}
//
//void Renderer::addText(const Font& font, const std::string& text, glm::vec2 pos, Color color)
//{
//	glm::vec2 size = font::measure(font, text);
//
//	glm::vec2 cursor = glm::vec2(0, size.y - font.base);
//	auto spaceInfo = font[' '];
//
//	auto itr = text.begin();
//	auto end   = text.end();
//
//	while(itr != end)
//	{
//		auto c = utf8::next(itr, end);
//		if(c == '\r') continue;
//
//			if(c == ' ') {
//				drawChar(font.page.texture, spaceInfo, pos, cursor, color);
//				cursor.x += spaceInfo.advance;
//				continue;
//			} else if(c == '\n') {
//				cursor.y -= font.lineHeight;
//				cursor.x = 0;
//				continue;
//			} else if(c == '\t') {
//				cursor += spaceInfo.advance * 4;
//			}
//
//			CharInfo info = font[c];
//			drawChar(font.page.texture, info, pos, cursor + info.offset, color);
//
//			cursor.x += info.advance;
//	}
//}
//
//void Renderer::addVertices(Texture texture, const Vertex* verts, size_t count)
//{
//	if( texture.glName != activeTexture.glName ||
//	    elements == capacity)
//	{
//		draw();
//		activeTexture = texture;
//	}
//
//	memcpy(&vertices[elements], verts, sizeof(Vertex) * count);
//}
//
//void Renderer::draw()
//{
//	if(elements == 0) return;
//
//	glUseProgram(program);
//	glUniform1i(samplerUniform, 0);
//	glUniformMatrix4fv(transformUniform, 1, false, &transform[0][0]);
//
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
//	glBindBuffer(GL_ARRAY_BUFFER, vbo);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, elements * sizeof(Vertex), &vertices[0]);
//
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, activeTexture.glName);
//
//	glEnableVertexAttribArray(posAttrib);
//	glEnableVertexAttribArray(colorAttrib);
//	glEnableVertexAttribArray(texcoordAttrib);
//
//	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
//	glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),(const void*)(sizeof(float) * 2));
//	glVertexAttribPointer(colorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 4));
//
//	glDrawElements(GL_TRIANGLES, (elements / 4) * 6, GL_UNSIGNED_SHORT, (void*)0);
//
//	elements = 0;
//
//	checkGlError("draw");
//}
