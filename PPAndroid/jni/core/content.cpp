/*
 * content.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: Gustav
 */

#include "content.h"
#include "image_loader.h"
#include "font_loading.h"
#include "path.h"

void obliterateFont(Font* font)
{
	font->obliterate();
}

void obliterateFrame(Frame* frame)
{
	frame->obliterate();
}

Content::Content(uint32_t max)
: fonts(&obliterateFont, max),
  frames(&obliterateFrame, max)
{
}

Frame internalLoadFrame(const std::string& path)
{
	LOGI("%s", path.c_str());
	ExternalAsset asset(path);
	auto tex = loadTexture(asset.buffer, asset.length);
	return Frame(tex, glm::vec4(0,1,1,-1));
}

uint32_t Content::loadFont(std::string fontPath)
{
	if (isFontLoaded(fontPath))
		return indexOfFont(fontPath);
	auto framePath = std::string(fontPath);
	framePath.erase(fontPath.size() - 4, 4);
	LOGI("%s", framePath.c_str());
	framePath += "_0.png";
	auto fi = loadFrame(framePath);
	ExternalAsset asset(fontPath);
	LOGI("Buff: %d, Len: %d", (uint32_t)asset.buffer, asset.length);
	auto font = constructFont(asset.buffer, asset.length, getFrame(fi));
	return fonts.add(fontPath, font);
}

uint32_t Content::loadFrame(std::string path)
{
	if (isFrameLoaded(path))
		return indexOfFrame(path);
	return frames.add(path, internalLoadFrame(path));
}

uint32_t Content::reloadFrame(std::string path)
{
	if (!isFrameLoaded(path))
		return loadFrame(path);
	return frames.replace(path, internalLoadFrame(path));
}

uint32_t Content::reloadFont(std::string fontPath)
{
	if (!isFontLoaded(fontPath))
		return loadFont(fontPath);

	ExternalAsset asset(fontPath);
	auto framePath = std::string(fontPath);
	framePath.erase(fontPath.size() - 4, 4);
	LOGI("%s", framePath.c_str());
	framePath += "_0.png";
	auto fi = reloadFrame(framePath);
	auto font = constructFont(asset.buffer, asset.length, getFrame(fi));
	return fonts.replace(fontPath, font);
}

bool Content::unloadFont(std::string fontPath)
{
	return fonts.remove(fontPath);
}

bool Content::unloadFont(uint32_t handle)
{
	return fonts.remove(handle);
}

bool Content::unloadFrame(std::string framePath)
{
	return frames.remove(framePath);
}

bool Content::unloadFrame(uint32_t handle)
{
	return frames.remove(handle);
}

void Content::unloadAll()
{
	frames.removeAll();
	fonts.removeAll();
}

uint32_t Content::indexOfFont(std::string path)
{
	return fonts.indexOf(path);
}

uint32_t Content::indexOfFrame(std::string path)
{
	return frames.indexOf(path);
}

bool Content::isFontLoaded(std::string path)
{
	return indexOfFont(path) != -1;
}

bool Content::isFrameLoaded(std::string path)
{
	return indexOfFrame(path) != -1;
}

void Content::reloadAsset(std::string path)
{
	if (path::hasExtension(path, FRAME_FILE_ENDING))
		reloadFrame(path);
	else if (path::hasExtension(path, FONT_FILE_ENDING))
		reloadFont(path);
	else
		ASSERTF(false, "Unrecognized file ending in path %s", path.c_str());
}


