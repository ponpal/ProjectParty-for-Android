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

	Font* loadFont(const char* path);
	void unloadFont(Font* font);
	Font* reloadFont(const char* path, Font* font);
}
//And more to come here i guess -- We only work with frames anyways


#endif /* RESOURCES_H_ */
