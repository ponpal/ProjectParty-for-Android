/*
 * loading_screen.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: Gustav
 */

#include "loading_screen.h"
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include "assert.h"
#include "image_loader.h"
#include "asset_helper.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>

LoadingScreen::LoadingScreen()
{
}

void LoadingScreen::load()
{
	LOGE("LoadingScreen load");
	Asset asset(LOADING_FRAME_PATH);
	auto tex = loadTexture(asset.buffer, asset.length);
	loadingFrame = Frame(tex);
}

void LoadingScreen::unload()
{
	glDeleteTextures(1, &loadingFrame.texture.glName);
}

void LoadingScreen::draw(Renderer* renderer, glm::vec2 dim)
{
	LOGE("LoadingScreen draw");
	glClearColor(0.5,0.5,0.5,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0,0, dim.x, dim.y);
	glm::mat4 matrix = glm::ortho(0.0f, dim.x, 0.0f, dim.y);
	vec2f p;
	p.x = p.y = 0;
	vec2f d;
	d.x = dim.x;
	d.y = dim.y;
	rendererAddFrame(renderer, &loadingFrame, p, d, 0xFFFFFFFF);
	rendererSetTransform(renderer, &matrix);
	rendererDraw(renderer);
}
