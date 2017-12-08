#include "UnitFactory.h"

namespace rwe
{
    UnitFactory::UnitFactory(
        UnitDatabase&& unitDatabase,
        MeshService&& meshService)
        : unitDatabase(std::move(unitDatabase)), meshService(std::move(meshService))
    {
    }

    Unit UnitFactory::createUnit(
        const std::string& unitType,
        PlayerId owner,
        unsigned int colorIndex,
        const Vector3f& position)
    {
        const auto& fbi = unitDatabase.getUnitInfo(unitType);
        const auto& soundClass = unitDatabase.getSoundClass(fbi.soundCategory);
        boost::optional<const MovementClass&> movementClass;
        if (!fbi.movementClass.empty())
        {
            movementClass = unitDatabase.getMovementClass(fbi.movementClass);
        }

        auto meshInfo = meshService.loadUnitMesh(fbi.objectName, colorIndex);

        const auto& script = unitDatabase.getUnitScript(fbi.unitName);
        auto cobEnv = std::make_unique<CobEnvironment>(&script);
        cobEnv->createThread("Create", std::vector<int>());
        Unit unit(meshInfo.mesh, std::move(cobEnv), std::move(meshInfo.selectionMesh));
        unit.owner = owner;
        unit.position = position;

        // These units are per-tick.
        // We divide by two here because TA ticks are 1/30 of a second,
        // where as ours are 1/60 of a second.
        unit.turnRate = (fbi.turnRate / 2.0f) * (Pif / 32768.0f); // also convert to rads
        unit.maxSpeed = fbi.maxVelocity / 2.0f;
        unit.acceleration = fbi.acceleration / 2.0f;
        unit.brakeRate = fbi.brakeRate / 2.0f;

        if (movementClass)
        {
            unit.footprintX = movementClass->footprintX;
            unit.footprintZ = movementClass->footprintZ;
        }
        else
        {
            unit.footprintX = fbi.footprintX;
            unit.footprintZ = fbi.footprintZ;
        }

        if (soundClass.select1)
        {
            unit.selectionSound = unitDatabase.getSoundHandle(*(soundClass.select1));
        }
        if (soundClass.ok1)
        {
            unit.okSound = unitDatabase.getSoundHandle(*(soundClass.ok1));
        }
        if (soundClass.arrived1)
        {
            unit.arrivedSound = unitDatabase.getSoundHandle(*(soundClass.arrived1));
        }

        return unit;
    }
}
