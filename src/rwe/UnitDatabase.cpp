#include "UnitDatabase.h"

namespace rwe
{

    const UnitFbi& UnitDatabase::getUnitInfo(const std::string& unitName) const
    {
        auto it = map.find(unitName);
        if (it == map.end())
        {
            throw std::runtime_error("No FBI data found for unit " + unitName);
        }

        return it->second;
    }

    void UnitDatabase::addUnitInfo(const std::string& unitName, const UnitFbi& info)
    {
        map.insert({unitName, info});
    }
}
