/*
 * font.cpp
 *
 *  Created on: Feb 13, 2014
 *      Author: Lukas_2
 */

#include "JNIHelper.h"
#include "font.h"
#include "types.h"
#include <stdexcept>
#include "buffer.h"

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
	Buffer buf;
	uint8_t* saved;

	Reader(uint8_t* _data, size_t _size)
	{
		buf.base = buf.ptr = _data;
		buf.length = _size;
		saved = buf.base;
	}

	uint8_t readByte()
	{
		return bufferReadByte(&buf);
	}

	void readBytes(uint8_t* ptr, size_t count)
	{
		bufferReadBytes(&buf, ptr, count);
	}

	uint32_t readInt()
	{
		return bufferReadInt(&buf);
	}

	void readInfoHeader(InfoHeader* ih)
	{
		bufferReadBytes(&buf, (uint8_t*)ih, sizeof(InfoHeader));
	}

	void readCommonHeader(CommonHeader* ch)
	{
		bufferReadBytes(&buf, (uint8_t*)ch, sizeof(CommonHeader));
	}

	uint32_t size()
	{
		return bufferBytesRemaining(&buf);
	}

	char* readName(uint32_t size)
	{
		auto p = buf.ptr;
		buf.ptr += size;
		return (char*)p;
	}

	CharRaw readCharRaw()
	{
		CharRaw r;
		bufferReadBytes(&buf, (uint8_t*)&r, sizeof(CharRaw));
		return r;
	}

	void savePosition()
	{
		saved = buf.ptr;
	}

	void resetPosition()
	{
		buf.ptr = saved;
	}

	void skip(uint32_t size)
	{
		buf.ptr += size;
	}
};

Font constructFont(uint8_t* data, size_t dataSize, const Frame& page)
{
	LOGI("Constructing font.");

	auto reader = Reader(data, dataSize);

		LOGI("while font.");
	if(reader.readByte() != 'B' ||
	   reader.readByte() != 'M' ||
	   reader.readByte() != 'F' ||
	   reader.readByte() != 3)
	{
		throw std::domain_error("Data supplied was not a BMF font");
	}

		LOGI("while font.");
	InfoHeader    iHeader;
	char*        fontName;
	size_t       fontNameLength;
	CommonHeader  cHeader;
	char*        pageName;
	size_t       pageNameLength;

	size_t       numCharacters;
	CharInfo*    chars;
	size_t		 max = 0;

	while(reader.size() > 0)
	{
		uint8_t type   = reader.readByte();
		uint32_t  size = reader.readInt();

		switch(type)
		{
			case BLOCKTYPE_INFO:
				reader.readInfoHeader(&iHeader);
				fontName = reader.readName(size - sizeof(InfoHeader));
				fontNameLength = size - sizeof(InfoHeader) - 1; //Excluding null terminator.
				break;
			case BLOCKTYPE_COMMON:
				reader.readCommonHeader(&cHeader);

				if(cHeader.pages != 1)
					throw std::domain_error("Currently fonts are only allowed to have one texture assosiated with them!");
				break;
			case BLOCKTYPE_PAGES:
				pageName = reader.readName(size);

				pageNameLength = size - 1;

				break;
			case BLOCKTYPE_CHARS:
				numCharacters = size / sizeof(CharRaw);
				reader.savePosition();
				size_t minChar;
				minChar = 0xFFFFFFFF;
				for(int i = 0; i < numCharacters; i++) {
					auto id = reader.readCharRaw().id;
					if(id < minChar) minChar = id;
					if(id > max) max = id;
				}
				reader.resetPosition();
				//Do we need the min? It could save some space but gives runtime lookup overhead.
				chars = new CharInfo[max + 1];

				for(int i = 0; i < max + 1; i++)
				{
					chars[i] = CharInfo();
				}

			    for(int i = 0; i < numCharacters; i++) {
			        auto r = reader.readCharRaw();

			        float advance  = r.xAdvance;
			        auto srcRect  = glm::vec4(r.x, r.y + r.height, r.width, -r.height);

			        auto offset   = glm::vec2(r.xOffset, cHeader.base - r.yOffset);
			        auto texCoord = glm::vec4(srcRect.x / cHeader.scaleW,
			                                 (srcRect.y + srcRect.w) / cHeader.scaleH,
			                                 (srcRect.x + srcRect.z) / cHeader.scaleW,
			                                  srcRect.y / cHeader.scaleH);

			        chars[r.id] = CharInfo(texCoord, srcRect, offset, advance);
			    }

				break;
			case BLOCKTYPE_KERNING_PAIRS:
				reader.skip(size);
				break;
			default:
				throw std::domain_error("Corrupt BMF file!");
		}
	}
	LOGI("No error");

	return Font(iHeader.fontSize, cHeader.base, cHeader.lineHeight, page, chars, max + 1);
}
