/*
 * path.h
 *
 *  Created on: Mar 3, 2014
 *      Author: Gustav
 */

#ifndef PATH_H_
#define PATH_H_

#include <string>

namespace path {
    bool hasExtension(const std::string& path, const std::string& extension);
    std::string withoutExtension(const std::string& path);
    std::string buildPath(const std::string& folder, const std::string& item);
    std::string changeExtension(const std::string& path, const std::string& newExtension);
    bool assetExists(const std::string& path);
}
#endif /* PATH_H_ */
