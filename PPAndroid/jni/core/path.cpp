/*
 * path.cpp
 *
 *  Created on: Mar 3, 2014
 *      Author: Gustav
 */

#include "path.h"

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
}
