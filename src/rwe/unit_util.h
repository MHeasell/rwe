#ifndef RWE_UNIT_UTIL_H
#define RWE_UNIT_UTIL_H

#include <string>
#include <vector>

namespace rwe
{
    void removeFromBuildQueue(std::vector<std::pair<std::string, int>>& buildQueue, const std::string& buildUnitType, int count);
}

#endif
