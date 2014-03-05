/*
 * new_renderer.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Lukas_2
 */


#include "new_renderer.h"
#include "types.h"
#include "shader.h"
#include <GLES2/gl2.h>
#include <cstdlib>
#include <cmath>
#include "font.h"
#include "external_libs/utf8.h"
#include "JNIHelper.h"


static const char gVertexShader[] =
    "attribute vec2 vPosition;\n"
	"attribute vec4 vColor;\n"
	"varying vec4 color;\n"
	"attribute vec2 vTexcoord;\n"
	"varying vec2 texcoord;\n"
	"uniform mat4 transform;"
	"void main() {\n"
	"  color       = vColor;\n"
	"  texcoord    = vTexcoord;\n"
    "  gl_Position = transform * vec4(vPosition,0,1);\n"
    "}\n";

static const char gFragmentShader[] =
    "precision mediump float;\n"
	"uniform sampler2D sampler;\n"
	"varying vec2 texcoord;\n"
	"varying vec4 color;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(sampler, texcoord) * color;\n"
    "}\n";

void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

GLuint createVBO(size_t count, size_t elementSize)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, count * elementSize, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOGI("Buffer Size: %d", count * elementSize);

    checkGlError("createVBO");
    return buffer;
}

GLuint createIBO(size_t count)
{
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);

	std::vector<GLushort> indices;
	indices.reserve(count * 6);
	for(GLushort i = 0; i < count * 4; i += 4) {
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 2);

		indices.push_back(i + 1);
		indices.push_back(i + 2);
		indices.push_back(i + 3);
	}

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * 6 * sizeof(short), &indices[0], GL_STATIC_DRAW);

	checkGlError("createIBO");
	return buffer;
}


typedef struct
{
	glm::vec2 position, texcoord;
	uint32_t color;
} Vertex;

struct Renderer
{
	Vertex* vertices;
	size_t elements, capacity;
	GLuint ibo, vbo, program;
	GLuint samplerUniform, transformUniform;
	GLuint posAttrib, colorAttrib, texcoordAttrib;
	glm::mat4 transform;
	Texture activeTexture;
};

Renderer* rendererInitialize(size_t maxBatchSize) //This is not yet usable.
{
	Renderer* renderer = new Renderer();
	size_t actualSize = maxBatchSize * 4;

	renderer->vertices = new Vertex[actualSize];
	renderer->elements = 0;
	renderer->capacity = actualSize;

	return renderer;
}

void rendererDelete(Renderer* renderer)
{
	delete renderer->vertices;
	delete renderer;
}

void rendererActivate(Renderer* renderer) //Must be called before any attempts to use the renderer!
{
	size_t actualSize = renderer->capacity;
	size_t maxBatchSize = actualSize / 4;

	renderer->vbo = createVBO(actualSize, sizeof(Vertex));
	renderer->ibo = createIBO(maxBatchSize);

	renderer->program = shader::createProgram(gVertexShader, gFragmentShader);
	renderer->samplerUniform 	= glGetUniformLocation(renderer->program, "sampler");
	renderer->transformUniform 	= glGetUniformLocation(renderer->program, "transform");

	renderer->posAttrib   		= glGetAttribLocation(renderer->program, "vPosition");
	renderer->colorAttrib 		= glGetAttribLocation(renderer->program, "vColor");
	renderer->texcoordAttrib 	= glGetAttribLocation(renderer->program, "vTexcoord");
}


void rendererSetTransform(Renderer* renderer, const glm::mat4* matrix)
{
	renderer->transform = (*matrix);
}

static void rendererFlushIfNeeded(Renderer* renderer, Texture tex, size_t count)
{
	if(renderer->activeTexture.glName != tex.glName ||
	   renderer->elements + count > renderer->capacity)
	{
		rendererDraw(renderer);
		renderer->activeTexture = tex;
	}
}


void rendererAddFrame(Renderer* renderer, const Frame* frame,
					  vec2f inPos, vec2f inDim, uint32_t color)
{
	rendererFlushIfNeeded(renderer, frame->texture, 4);
	using namespace glm;

	vec2 bl  = vec2(inPos.x, inPos.y);
	vec2 dim = vec2(inDim.x, inDim.y);

	vec2 bottomLeft = vec2(frame->coords.x, frame->coords.y);
	vec2 topRight   = vec2(frame->coords.z, frame->coords.w);

	auto ptr = &(renderer->vertices[renderer->elements]);

	Vertex vert;
	vert.position = bl;
	vert.texcoord = bottomLeft;
	vert.color    = color;

	*(ptr++) = vert;
	vert.position = bl + vec2(dim.x, 0);
	vert.texcoord = bottomLeft + vec2(topRight.x, 0);
	*(ptr++) = vert;

	vert.position = bl + vec2(0, dim.y);
	vert.texcoord = bottomLeft + vec2(0, topRight.y);
	*(ptr++) = vert;

	vert.position = bl + dim;
	vert.texcoord = bottomLeft + topRight;
	*(ptr++) = vert;

	renderer->elements += 4;
}

static inline glm::vec2 rotated(glm::vec2 pos, glm::vec2 offset, float sinus, float cosinus)
{
	pos.x += offset.x * cosinus - offset.y * sinus;
	pos.y += offset.x * sinus   + offset.y * cosinus;
	return pos;
}

void rendererAddFrame2(Renderer* renderer, const Frame* frame,
					   vec2f inPos, vec2f inDim, uint32_t color,
					   vec2f inOrigin, float rotation, int mirrored)
{
	rendererFlushIfNeeded(renderer, frame->texture, 4);

	using namespace glm;

	vec2 pos = vec2(inPos.x, inPos.y),
		 dim = vec2(inDim.x, inDim.y),
		 origin = vec2(inOrigin.x, inOrigin.y);


	vec2 botLeft  = vec2(frame->coords.x, frame->coords.y);
	vec2 topRight = vec2(frame->coords.z + frame->coords.x,
								   frame->coords.w + frame->coords.y);

	if(mirrored)
	{
		float tmp = botLeft.x;
		botLeft.x = topRight.x;
		topRight.x = tmp;
	}

	Vertex vert;
	vert.color    = color;

	float sinus = sin(rotation),
		cosinus = cos(rotation);

	auto ptr = &(renderer->vertices[renderer->elements]);

	vert.position = rotated(pos, -origin, sinus, cosinus);
	vert.texcoord = botLeft;
	*(ptr++) = vert;

	vert.position = rotated(pos, -origin + glm::vec2(0, dim.y), sinus, cosinus);
	vert.texcoord = glm::vec2(botLeft.x, topRight.y);
	*(ptr++) = vert;

	vert.position = rotated(pos, -origin + glm::vec2(dim.x, 0), sinus, cosinus);
	vert.texcoord = glm::vec2(topRight.x, botLeft.y);
	*(ptr++) = vert;

	vert.position = rotated(pos, -origin + dim, sinus, cosinus);
	vert.texcoord = topRight;
	*(ptr++) = vert;

	renderer->elements += 4;
}

static inline void rendererDrawChar(Renderer* renderer, Texture texture, const CharInfo& info,
									glm::vec2 pos, glm::vec2 offset, uint32_t color)
{
	rendererFlushIfNeeded(renderer, texture, 4);
	auto ptr = &(renderer->vertices[renderer->elements]);


	glm::vec2 location   = pos + offset;
	glm::vec2 bottomLeft = glm::vec2(info.textureCoords.x, info.textureCoords.y);
	glm::vec2 topRight   = glm::vec2(info.textureCoords.z, info.textureCoords.w);
	glm::vec2 dim        = glm::vec2(info.srcRect.z, info.srcRect.w);

	Vertex vert;
	vert.position = location;
	vert.texcoord = bottomLeft;
	vert.color    = color;
	*(ptr++) = vert;

	vert.position = location + glm::vec2(dim.x, 0);
	vert.texcoord = glm::vec2(topRight.x, bottomLeft.y);
	*(ptr++) = vert;

	vert.position = location + glm::vec2(0, dim.y);
	vert.texcoord = glm::vec2(bottomLeft.x, topRight.y);;
	*(ptr++) = vert;

	vert.position = location + dim;
	vert.texcoord = topRight;
	*(ptr++) = vert;

	renderer->elements += 4;
}

inline static glm::vec2 measureFont(const Font& font, const char* text)
{
	float width = 0, height = 0, cursor = 0;

	auto spaceInfo = font[' '];
	auto itr   = &text[0];
	auto end   = &text[strlen(text)];

	while(itr != end)
	{
		auto c = utf8::next(itr, end);
		if(c == '\r') continue;

		if(c == ' ') {
			cursor += spaceInfo.advance;
			continue;
		} else if(c == '\n') {
			height += font.lineHeight;
			width  = fmaxf(cursor, width);
			cursor = 0;
			continue;
		} else if(c == '\t') {
			cursor += spaceInfo.advance * 4;
		}

		CharInfo info = font.chars[c];
		cursor += info.advance;
	}

	width = fmaxf(width, cursor);
	height += font.base;
	return glm::vec2(width, height);
}

void rendererAddText(Renderer* renderer, const Font* font, const char* text, vec2f inPos, uint32_t color)
{
	using namespace glm;
	vec2 pos = vec2(inPos.x, inPos.y);
	vec2 size = measureFont(*font, text);
	vec2 cursor = vec2(0, size.y - font->base);
	CharInfo spaceInfo = (*font)[' '];

	auto itr   = &text[0];
	auto end   = &text[strlen(text)];

	while(itr != end)
	{
		auto c = utf8::next(itr, end);
		if(c == '\r') continue;

			if(c == ' ') {
				rendererDrawChar(renderer, font->page.texture, spaceInfo, pos, cursor, color);
				cursor.x += spaceInfo.advance;
				continue;
			} else if(c == '\n') {
				cursor.y -= font->lineHeight;
				cursor.x = 0;
				continue;
			} else if(c == '\t') {
				cursor += spaceInfo.advance * 4;
			}

			CharInfo info = (*font)[c];
			rendererDrawChar(renderer, font->page.texture, info, pos, cursor + info.offset, color);

			cursor.x += info.advance;
	}
}


void rendererDraw(Renderer* renderer)
{
	if(renderer->elements == 0) return;

	glUseProgram(renderer->program);
	glUniform1i(renderer->samplerUniform, 0);
	glUniformMatrix4fv(renderer->transformUniform, 1, false, &(renderer->transform[0][0]));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->elements * sizeof(Vertex), &(renderer->vertices[0]));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->activeTexture.glName);

	glEnableVertexAttribArray(renderer->posAttrib);
	glEnableVertexAttribArray(renderer->colorAttrib);
	glEnableVertexAttribArray(renderer->texcoordAttrib);

	glVertexAttribPointer(renderer->posAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	glVertexAttribPointer(renderer->texcoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),(const void*)(sizeof(float) * 2));
	glVertexAttribPointer(renderer->colorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 4));

	glDrawElements(GL_TRIANGLES, (renderer->elements / 4) * 6, GL_UNSIGNED_SHORT, (void*)0);

	renderer->elements = 0;

	checkGlError("draw");

}
