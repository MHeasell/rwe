#ifndef RWE_UNITFACTORY_H
#define RWE_UNITFACTORY_H

#include <rwe/MeshService.h>
#include <rwe/MovementClass.h>
#include <rwe/MovementClassCollisionService.h>
#include <rwe/Unit.h>
#include <rwe/UnitDatabase.h>
#include <string>

namespace rwe
{
    class UnitFactory
    {
    private:
        UnitDatabase unitDatabase;
        MeshService meshService;
        MovementClassCollisionService* const collisionService;

    public:
        UnitFactory(UnitDatabase&& unitDatabase, MeshService&& meshService, MovementClassCollisionService* collisionService);

    public:
        Unit createUnit(const std::string& unitType, PlayerId owner, unsigned int colorIndex, const Vector3f& position);
    };
}

#endif
