/*
 * path.cpp
 *
 *  Created on: Mar 3, 2014
 *      Author: Gustav
 */

#include "path.h"
#include <string>

namespace path {
    bool hasExtension(const std::string& str, const std::string& ending)
    {
        if (str.length() >= ending.length()) {
            return (0 == str.compare(str.length() - ending.length(),
                    ending.length(),ending));
        } else {
            return false;
        }
    }

    std::string withoutExtension(const std::string& path)
    {
    	auto index = path.find_last_of('.', path.size());
    	return path.substr(0, index);
    }

    std::string buildPath(const std::string& folder, const std::string& item)
    {
    	std::string s(folder);
    	s += "/" + item;
    	return s;
    }

    std::string changeExtension(const std::string& path, const std::string& newExtension)
    {
    	auto index = path.find_last_of('.', path.size());
    	return path.substr(0, index) + newExtension;
    }
}
