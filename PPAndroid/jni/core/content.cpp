/*
 * content.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: Gustav
 */

#include "content.h"
#include "image_loader.h"
#include "font_loading.h"
#include "log.h"

Frame internalLoadFrame(const std::string& path)
{
	ExternalAsset asset(path);
	auto tex = loadTexture(asset.buffer, asset.length);
	return Frame(tex, glm::vec4(0,1,1,-1));
}

uint32_t Content::loadFont(std::string fontPath)
{
	if (isFontLoaded(fontPath))
		return indexOfFont(fontPath);
	ExternalAsset asset(fontPath);
	auto framePath = std::string(fontPath);
	framePath.erase(fontPath.size() - 4, 4);
	LOGI(framePath.c_str());
	framePath += "_0.png";
	auto fi = loadFrame(framePath);
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
	LOGI(framePath.c_str());
	framePath += "_0.png";
	auto fi = reloadFrame(framePath);
	auto font = constructFont(asset.buffer, asset.length, getFrame(fi));
	return fonts.replace(fontPath, font);
}

bool Content::unloadFont(std::string fontPath)
{
	return fonts.remove(fontPath);
}

bool Content::unloadFrame(std::string framePath)
{
	return frames.remove(framePath);
}

uint32_t Content::indexOfFont(std::string path)
{
	return fonts.indexOf(path);
}

uint32_t Content::indexOfFrame(std::string path)
{
	return frames.indexOf(path);
}
