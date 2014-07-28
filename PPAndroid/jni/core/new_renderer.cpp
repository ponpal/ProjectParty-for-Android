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
#include "assert.h"

#define FRAME_EXTRA  0

static const char gVertexShader[] =
"attribute vec2 vPosition;\n"
"attribute vec4 vColor;\n"
"attribute vec2 vTexcoord;\n"
"attribute vec4 vExtra;\n"
"\n"
"varying vec2 texcoord;\n"
"varying vec4 color;\n"
"varying vec4 extra;\n"
"\n"
"uniform mat4 transform;\n"
"void main() \n"
"{\n"
"  color       = vColor;\n"
"  texcoord    = vTexcoord;\n"
"  extra	   = vec4(vExtra.x, vExtra.yzw / 255.0);\n"
"  gl_Position = transform * vec4(vPosition ,0 ,1 );\n"
"}\n";

static const char gFragmentShader[] =
"precision mediump float;\n"
"uniform sampler2D sampler;\n"
"varying vec2 texcoord;\n"
"varying vec4 color;\n"
"varying vec4 extra;\n"
"void main() {\n"
"	vec4 sample = texture2D(sampler, texcoord);\n"
"	vec4 result = vec4(1,1,1,1);\n"
"	int typ = int(extra.x);\n"
"	if(typ == 0) \n"
"	{\n"
"		result = sample * color;\n"
"	} \n"
"	else \n"
"	{\n"
"		float s = sample[typ - 1];"
"		if(s < extra.y) \n"
"			discard; \n"
"	 \n"
"	    result = color; \n"
"	    result.a *= smoothstep(extra.y, extra.z, s);\n"
"	}\n"
"\n"
"	gl_FragColor = result;\n"
"}\n";

void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        RLOGI("after %s() glError (0x%x)\n", op, error);
    }
}

GLuint createVBO(size_t count, size_t elementSize)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, count * elementSize, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    RLOGI("Buffer Size: %d", count * elementSize);

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
	uint32_t color, extra;
} Vertex;

struct Renderer
{
	Vertex* vertices;
	size_t elements, capacity;
	GLuint ibo, vbo, program;
	GLuint samplerUniform, transformUniform;
	GLuint posAttrib, colorAttrib, texcoordAttrib, extraAttrib;
	glm::mat4 transform;
	Texture activeTexture;
};

static void rendererActivate(Renderer* renderer) //Must be called before any attempts to use the renderer!
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
	renderer->extraAttrib       = glGetAttribLocation(renderer->program, "vExtra");
}

Renderer* rendererCreate(size_t maxBatchSize) //This is not yet usable.
{
	ASSERT(maxBatchSize*4 < 0xFFFF, "Batchsize cannot fit in Glushort, please use a value that is less than 16k");
	Renderer* renderer = new Renderer();
	size_t actualSize = maxBatchSize * 4;

	renderer->vertices = new Vertex[actualSize];
	renderer->elements = 0;
	renderer->capacity = actualSize;
	rendererActivate(renderer);
	return renderer;
}

void rendererDestroy(Renderer* renderer)
{
	delete [] renderer->vertices;
	delete renderer;
}

void rendererSetTransform(Renderer* renderer, matrix4 matrix)
{
	renderer->transform = *((glm::mat4*) &matrix);
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
	if(frame == nullptr) {
		RLOGE("%s", "rendererAddFrame called with null frame!");
		return;
	}

	rendererFlushIfNeeded(renderer, frame->texture, 4);
	using namespace glm;
	vec2 bl  = vec2(inPos.x, inPos.y);
	vec2 dim = vec2(inDim.x, inDim.y);

	vec2 bottomLeft = vec2(frame->x, frame->y);
	vec2 topRight   = vec2(frame->width , frame->height);

	auto ptr = &(renderer->vertices[renderer->elements]);

	Vertex vert;
	vert.extra    = FRAME_EXTRA;
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
	if(frame == nullptr) {
		RLOGE("%s", "rendererAddFrame2 called with null frame!");
		return;
	}

	rendererFlushIfNeeded(renderer, frame->texture, 4);

	using namespace glm;

	vec2 pos = vec2(inPos.x, inPos.y),
		 dim = vec2(inDim.x, inDim.y),
		 origin = vec2(inOrigin.x, inOrigin.y);


	vec2 botLeft  = vec2(frame->x, frame->y);
	vec2 topRight = vec2(frame->width, frame->height);

	if(mirrored)
	{
		botLeft.x = botLeft.x + topRight.x;
		topRight.x = - topRight.x;
	}

	Vertex vert;
	vert.extra	  = FRAME_EXTRA;
	vert.color    = color;

	float sinus = sin(rotation),
		cosinus = cos(rotation);

	auto ptr = &(renderer->vertices[renderer->elements]);

	vert.position = rotated(pos, -origin, sinus, cosinus);
	vert.texcoord = botLeft;
	*(ptr++) = vert;

	vert.position = rotated(pos, -origin + glm::vec2(0, dim.y), sinus, cosinus);
	vert.texcoord = botLeft + glm::vec2(topRight.x,0);
	*(ptr++) = vert;

	vert.position = rotated(pos, -origin + glm::vec2(dim.x, 0), sinus, cosinus);
	vert.texcoord = botLeft + glm::vec2(0, topRight.y);
	*(ptr++) = vert;

	vert.position = rotated(pos, -origin + dim, sinus, cosinus);
	vert.texcoord = botLeft + topRight;
	*(ptr++) = vert;

	renderer->elements += 4;
}

static inline void rendererDrawChar(Renderer* renderer, Texture texture, const CharInfo* info,
									glm::vec2 pos, glm::vec2 offset, uint32_t color, glm::vec2 scale,
									uint32_t extra)
{
	rendererFlushIfNeeded(renderer, texture, 4);
	auto ptr = &(renderer->vertices[renderer->elements]);


	glm::vec2 location   = (pos + offset);
	glm::vec2 bottomLeft = glm::vec2(info->textureCoords.x, info->textureCoords.y);
	glm::vec2 topRight   = glm::vec2(info->textureCoords.z, info->textureCoords.w);
	glm::vec2 dim        = glm::vec2(info->srcRect.z, info->srcRect.w) * scale;


	Vertex vert;
	vert.extra	  = extra;
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

inline static int measureFont(const Font* font, const char* text)
{
	int count = 0;
	for(;*text; text++)
		if(*text == '\n') count++;

	return count;
}

void rendererAddText(Renderer* renderer, const Font* font,
					 const char* text, vec2f inPos,
					 uint32_t color, vec2f dim, vec2f thresholds)
{
	if(font == nullptr) {
		RLOGE("%s", "rendererAddText called with null font.");
		return;
	} else if(text == nullptr)
	{
		RLOGE("%s", "rendererAddText called with null text");
		return;
	}


	uint32_t extra = (font->layer + 1) |
					(((uint32_t)(thresholds.x * 255.0f) & 0xFF) << 8) |
					(((uint32_t)(thresholds.y * 255.0f) & 0xFF) << 16);
					//((uint32_t)(0xFF) << 24);

	glm::vec2 scale(dim.x / font->size, dim.y  / font->size);

	using namespace glm;
	auto pos = glm::vec2(inPos.x, inPos.y);
	int count = measureFont(font, text);
	auto cursor = glm::vec2(0,  -font->size * scale.y + font->lineHeight * count * scale.y);

	auto spaceInfo = fontCharInfo(font, ' ');


	auto itr   = &text[0];
	auto end   = &text[strlen(text)];

	while(itr != end)
	{
		auto c = utf8::next(itr, end);
		if(c == '\r') continue;

			if(c == ' ') {
				cursor.x += spaceInfo->advance * scale.x;
				continue;
			} else if(c == '\n') {
				cursor.y -= font->lineHeight * scale.y;
				cursor.x = 0;
				continue;
			} else if(c == '\t') {
				cursor.x += spaceInfo->advance * 4 * scale.x;
				continue;
			}

			auto info = fontCharInfo(font, c);
			glm::vec2 offset = glm::vec2(info->offset.x, info->offset.y) * scale;
			rendererDrawChar(renderer, font->page, info, pos, cursor + offset, color, scale, extra);

			cursor.x += info->advance * scale.x;
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
	glEnableVertexAttribArray(renderer->extraAttrib);

	glVertexAttribPointer(renderer->posAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	glVertexAttribPointer(renderer->texcoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),(const void*)(sizeof(float) * 2));
	glVertexAttribPointer(renderer->colorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 4));
	glVertexAttribPointer(renderer->extraAttrib, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(float) * 4 + sizeof(uint32_t)));

	glDrawElements(GL_TRIANGLES, (renderer->elements / 4) * 6, GL_UNSIGNED_SHORT, (void*)0);

	renderer->elements = 0;

	checkGlError("draw");
}
