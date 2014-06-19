/*
 * asset_helper.cpp
 *
 *  Created on: Mar 3, 2014
 *      Author: Gustav
 */

#include "asset_helper.h"
#include "android/asset_manager.h"
#include "unistd.h"

bool assetExists(const char* filePath)
{
	auto res = access(filePath, F_OK);
	return res == 0;
}

Asset::Asset(const char* fileName)
{
	auto mgr  = gApp->activity->assetManager;
	asset = AAssetManager_open(mgr, fileName, AASSET_MODE_RANDOM);
	length = AAsset_getLength(asset);
	buffer = (uint8_t*)AAsset_getBuffer(asset);
}

Asset::~Asset()
{
	AAsset_close(asset);
}

ExternalAsset::ExternalAsset(const std::string& filePath)
{
    LOGI("Trying to load file %s", filePath.c_str());
    auto file = fopen(filePath.c_str(), "r+");
    ASSERTF(file != NULL, "Couldn't open file. %s", filePath.c_str());


    fseek( file, 0L, SEEK_END);
    length = ftell(file);
    rewind(file);
    buffer = new uint8_t[length];

    fread(buffer, length, 1, file);
    fclose(file);
    LOGI("Successfully loaded!");
}

ExternalAsset::~ExternalAsset()
{
    delete buffer;
}
