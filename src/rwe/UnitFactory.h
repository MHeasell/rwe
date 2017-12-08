#ifndef RWE_UNITFACTORY_H
#define RWE_UNITFACTORY_H

#include <rwe/Unit.h>
#include <string>
#include <rwe/MovementClass.h>
#include <rwe/UnitDatabase.h>
#include <rwe/MeshService.h>

namespace rwe
{
    class UnitFactory
    {
    private:
        UnitDatabase unitDatabase;
        MeshService meshService;

    public:
        UnitFactory(UnitDatabase&& unitDatabase, MeshService&& meshService);

    public:
        Unit createUnit(const std::string& unitType, PlayerId owner, unsigned int colorIndex, const Vector3f& position);
    };
}

#endif
