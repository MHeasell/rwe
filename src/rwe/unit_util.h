#ifndef RWE_UNIT_UTIL_H
#define RWE_UNIT_UTIL_H

#include <string>
#include <vector>
#include <unordered_map>

namespace rwe
{
    void removeFromBuildQueue(std::vector<std::pair<std::string, int>>& buildQueue, const std::string& buildUnitType, int count);

    std::unordered_map<std::string, int> getBuildQueueTotalsStatic(const std::vector<std::pair<std::string, int>>& buildQueue);
}

#endif
