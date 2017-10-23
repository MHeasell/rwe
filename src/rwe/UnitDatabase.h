#ifndef RWE_UNITDATABASE_H
#define RWE_UNITDATABASE_H

#include "UnitFbi.h"

namespace rwe
{
    class UnitDatabase
    {
    private:
        std::unordered_map<std::string, UnitFbi> map;

    public:
        const UnitFbi& getUnitInfo(const std::string& unitName) const;

        void addUnitInfo(const std::string& unitName, const UnitFbi& info);
    };
}

#endif
