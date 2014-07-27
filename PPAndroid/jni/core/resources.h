/*
 * resources.h
 *
 *  Created on: 18 jul 2014
 *      Author: Lukas
 */

#ifndef RESOURCES_H_
#define RESOURCES_H_

extern "C"
{
	Texture loadTexture(const char* path);
	void unloadTexture(Texture tex);
	Texture reloadTexture(const char* path, Texture tex);

	FontAtlas* loadFont(const char* path);
	void unloadFont(FontAtlas* font);
	FontAtlas* reloadFont(const char* path, FontAtlas* font);
}
//And more to come here i guess -- We only work with frames anyways


#endif /* RESOURCES_H_ */
