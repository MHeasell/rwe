#pragma once

#include <deque>
#include <string>
#include <unordered_map>

namespace rwe
{
    void removeFromBuildQueue(std::deque<std::pair<std::string, int>>& buildQueue, const std::string& buildUnitType, int count);

    std::unordered_map<std::string, int> getBuildQueueTotalsStatic(const std::deque<std::pair<std::string, int>>& buildQueue);
}
