/*
 * content.h
 *
 *  Created on: Feb 26, 2014
 *      Author: Gustav
 */

#ifndef CONTENT_H_
#define CONTENT_H_

#include "JNIHelper.h"
#include "resource_table.h"
#include "types.h"
#include "font.h"
#include <string>

struct Content
{
	ResourceTable<Frame> frames;
	ResourceTable<Font> fonts;

	uint32_t loadFrame(std::string path);
	uint32_t loadFont(std::string path);

	uint32_t reloadFrame(std::string path);
	uint32_t reloadFont(std::string path);

	bool unloadFrame(std::string path);
	bool unloadFont(std::string path);

	bool isFrameLoaded(std::string path);
	bool isFontLoaded(std::string path);

	uint32_t indexOfFrame(std::string path);
	uint32_t indexOfFont(std::string path);

	inline const Font& getFont(uint32_t index)
	{
		return fonts[index];
	}

	inline const Frame& getFrame(uint32_t index)
	{
		return frames[index];
	}
};



#endif /* CONTENT_H_ */
