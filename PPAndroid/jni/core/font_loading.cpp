/*
 * font.cpp
 *
 *  Created on: Feb 13, 2014
 *      Author: Lukas_2
 */

#include "font.h"
#include "types.h"
#include <stdexcept>
#include "log.h"

#pragma pack(1)
struct InfoHeader
{
	uint16_t fontSize;
	uint8_t bitField;
	uint8_t charSet;
	uint16_t stretchH;
	uint8_t aa;
	uint8_t paddingUp;
	uint8_t paddingRight;
	uint8_t paddingDown;
	uint8_t paddingLeft;
	uint8_t spacingHoriz;
	uint8_t spacingVert;
	uint8_t outline;
};

#pragma pack(1)
struct CommonHeader
{
	uint16_t lineHeight;
	uint16_t base;
	uint16_t scaleW;
	uint16_t scaleH;
	uint16_t pages;
	uint8_t bitField;
	uint8_t alphaChan;
	uint8_t redChan;
	uint8_t greenChan;
	uint8_t blueChan;
};

struct CharRaw
{
	uint32_t id;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	uint16_t xOffset;
	uint16_t yOffset;
	uint16_t xAdvance;
	uint8_t page;
	uint8_t chan;
};

#pragma pack(1)
struct KerningPair
{
	uint32_t first;
	uint32_t second;
	uint16_t amount;
};

#define BLOCKTYPE_INFO 1
#define BLOCKTYPE_COMMON 2
#define BLOCKTYPE_PAGES 3
#define BLOCKTYPE_CHARS 4
#define BLOCKTYPE_KERNING_PAIRS 5

struct Reader
{
	uint8_t* data;
	size_t size;

	Reader(uint8_t* _data, size_t _size)
	: data(_data), size(_size)
	{ }

	uint8_t readByte()
	{
		uint8_t byte = *data++;
		size--;
		return byte;
	}

	uint8_t* readBytes(size_t count)
	{
		uint8_t* bytes = data;
		data += count;
		size -= count;
		return bytes;
	}

	uint16_t readShort()
	{
		//Add some native decoding here maby?
		uint16_t result = *(uint32_t*)data;
		data += sizeof(uint16_t);
		size -= sizeof(uint16_t);
		return result;
	}

	uint32_t readInt()
	{
		uint32_t result = *((uint32_t*)data);
		data += sizeof(uint32_t);
		size -= sizeof(uint32_t);
		return result;
	}
};

Font constructFont(uint8_t* data, size_t dataSize, const Frame& page)
{
	LOGI("Constructing font.");

	auto reader = Reader(data, dataSize);

	if(reader.readByte() != 'B' ||
	   reader.readByte() != 'M' ||
	   reader.readByte() != 'F' ||
	   reader.readByte() != 3)
	{
		throw std::domain_error("Data supplied was not a BMF font");
	}

	InfoHeader    iHeader;
	char*        fontName;
	size_t       fontNameLength;
	CommonHeader  cHeader;
	char*        pageName;
	size_t       pageNameLength;

	CharRaw*     rawCharInfo;
	size_t       numCharacters;

	KerningPair* kerningInfo;
	size_t       numKernings;



	while(reader.size > 0)
	{
		uint8_t type   = reader.readByte();
		uint32_t  size = reader.readInt();

		switch(type)
		{
			case BLOCKTYPE_INFO:
				iHeader = *(InfoHeader*)reader.readBytes(sizeof(InfoHeader));
				fontName = (char*)reader.readBytes(size - sizeof(InfoHeader));
				fontNameLength = size - sizeof(InfoHeader) - 1; //Excluding null terminator.
				break;
			case BLOCKTYPE_COMMON:
				cHeader = *(CommonHeader*)reader.readBytes(sizeof(CommonHeader));

				if(cHeader.pages != 1)
					throw std::domain_error("Currently fonts are only allowed to have one texture assosiated with them!");
				break;
			case BLOCKTYPE_PAGES:
				pageName = (char*)reader.readBytes(size);
				pageNameLength = size - 1;

				break;
			case BLOCKTYPE_CHARS:
				rawCharInfo   = (CharRaw*)reader.readBytes(size);
				numCharacters = size / sizeof(CharRaw);
				break;
			case BLOCKTYPE_KERNING_PAIRS:
				kerningInfo   = (KerningPair*)reader.readBytes(size);
				numKernings = size / sizeof(KerningPair);
				break;
			default:
				throw std::domain_error("Corrupt BMF file!");
		}
	}

	size_t min = 0xFFFFFFFF, max = 0;
	for(int i = 0; i < numCharacters; i++) {
		auto id = rawCharInfo[i].id;
		if(id < min) min = id;
		if(id > max) max = id;
	}

	//Do we need the min? It could save some space but gives runtime lookup overhead.
	CharInfo* chars = new CharInfo[max + 1];
	for(int i = 0; i < max + 1; i++)
	{
		chars[i] = CharInfo();
	}

	for(int i = 0; i < numCharacters; i++) {
		auto r = rawCharInfo[i];

		float advance  = r.xAdvance;
		auto srcRect  = glm::vec4(r.x, r.y + r.height, r.width, -r.height);

		auto offset   = glm::vec2(r.xOffset, cHeader.base - r.yOffset);
		auto texCoord = glm::vec4(srcRect.x / cHeader.scaleW,
								 (srcRect.y + srcRect.w) / cHeader.scaleH,
								 (srcRect.x + srcRect.z) / cHeader.scaleW,
								  srcRect.y / cHeader.scaleH);

		chars[r.id] = CharInfo(texCoord, srcRect, offset, advance);
	}

	return Font(iHeader.fontSize, cHeader.base, cHeader.lineHeight, page, chars, max + 1);
}
