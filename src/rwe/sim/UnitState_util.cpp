#include "UnitState_util.h"

namespace rwe
{
    void removeFromBuildQueue(std::deque<std::pair<std::string, int>>& buildQueue, const std::string& buildUnitType, int count)
    {
        auto it = buildQueue.end();
        while (count > 0 && it != buildQueue.begin())
        {
            --it;

            auto& elem = *it;
            if (elem.first != buildUnitType)
            {
                continue;
            }

            if (count < elem.second)
            {
                elem.second -= count;
                break;
            }

            count -= elem.second;
            it = buildQueue.erase(it);
        }
    }

    std::unordered_map<std::string, int> getBuildQueueTotalsStatic(const std::deque<std::pair<std::string, int>>& buildQueue)
    {
        std::unordered_map<std::string, int> map;
        for (const auto& e : buildQueue)
        {
            auto it = map.find(e.first);
            if (it == map.end())
            {
                map.insert(e);
                continue;
            }

            it->second += e.second;
        }

        return map;
    }
}
