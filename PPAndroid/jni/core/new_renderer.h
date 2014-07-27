/*
 * new_renderer.h
 *
 *  Created on: Feb 25, 2014
 *      Author: Lukas_2
 */

#ifndef NEW_RENDERER_H_
#define NEW_RENDERER_H_

#include "types.h"
#include "font.h"

extern "C" {

typedef struct Renderer Renderer;

Renderer* rendererCreate(size_t batchSize);
void rendererDestroy(Renderer* renderer);
void rendererSetTransform(Renderer* renderer, matrix4 transform);
void rendererAddFrame(Renderer* renderer, const Frame* frame,
					 vec2f pos, vec2f dim, uint32_t color);
void rendererAddFrame2(Renderer* renderer, const Frame* frame,
		vec2f pos, vec2f dim, uint32_t color,
		vec2f origin, float rotation, int mirrored);
void rendererAddText(Renderer* renderer, const Font* font,
					 const char* text, vec2f inPos,
					 uint32_t color, vec2f dim, vec2f thresholds);

void rendererDraw(Renderer* renderer); // Force a draw.

}

#endif /* NEW_RENDERER_H_ */
