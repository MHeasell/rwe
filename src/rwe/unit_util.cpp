#include "unit_util.h"

namespace rwe
{
    void removeFromBuildQueue(std::vector<std::pair<std::string, int>>& buildQueue, const std::string& buildUnitType, int count)
    {
        auto it = buildQueue.rbegin();
        auto end = buildQueue.rend();
        while (count > 0 && it != end)
        {
            auto& elem = *it;
            if (elem.first != buildUnitType)
            {
                ++it;
                continue;
            }

            if (count < elem.second)
            {
                elem.second -= count;
                break;
            }

            buildQueue.erase(it.base());
            count -= elem.second;

            ++it;
        }
    }

    std::unordered_map<std::string, int> getBuildQueueTotalsStatic(const std::vector<std::pair<std::string, int>>& buildQueue)
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
