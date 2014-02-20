/*
 * renderer.h
 *
 *  Created on: Feb 11, 2014
 *      Author: Lukas_2
 */

#ifndef RENDERER_H_
#define RENDERER_H_

#include "types.h"
#include "font.h"
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <string>

struct Vertex
{
	glm::vec2 position, texcoord;
	Color color;

	Vertex() { }
	Vertex(const Vertex& other)
	 :position(other.position),
	  texcoord(other.texcoord),
	  color(other.color) { }

};

class Renderer
{
public:
	Renderer(size_t batchSize);
	~Renderer();
	void setTransform(const glm::mat4& matrix);
	void addFrame(const Frame& frame, glm::vec2 pos, glm::vec2 dim, Color color);
	void addFrame(const Frame& frame, glm::vec2 pos, glm::vec2 dim,
				  Color color, glm::vec2 origin, float rotation, bool mirrored);
	void addText(const Font& font, const std::string& text, glm::vec2 pos, Color color);
	void addVertices(Texture texture, const Vertex* vertex, size_t count);
	void draw();

private:
	void flushIfNeeded(Texture texture, size_t count);
	void drawChar(Texture texture, const CharInfo& info, glm::vec2 pos, glm::vec2 offset, Color color);

	Vertex* vertices;
	size_t elements, capacity;
	GLuint ibo, vbo, program;
	GLuint samplerUniform, transformUniform;
	GLuint posAttrib, colorAttrib, texcoordAttrib;
	glm::mat4 transform;
	Texture activeTexture;
};


//void drawFrame(const Frame& frame, float2 pos, Color color);
//void drawText(const Font& font, const char* text, float2 pos, Color color);

#endif /* RENDERER_H_ */
