#ifndef RWE_UNITDATABASE_H
#define RWE_UNITDATABASE_H

#include <rwe/Cob.h>
#include "UnitFbi.h"

namespace rwe
{
    class UnitDatabase
    {
    private:
        std::unordered_map<std::string, UnitFbi> map;

        std::unordered_map<std::string, CobScript> cobMap;

    public:
        const UnitFbi& getUnitInfo(const std::string& unitName) const;

        void addUnitInfo(const std::string& unitName, const UnitFbi& info);

        const CobScript& getUnitScript(const std::string& unitName) const;

        void addUnitScript(const std::string& unitName, CobScript&& cob);
    };
}

#endif
