#include "UnitFactory.h"

namespace rwe
{
    UnitFactory::UnitFactory(
        UnitDatabase&& unitDatabase,
        MeshService&& meshService,
        MovementClassCollisionService* collisionService)
        : unitDatabase(std::move(unitDatabase)), meshService(std::move(meshService)), collisionService(collisionService)
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
        std::optional<std::reference_wrapper<const MovementClass>> movementClassOption;
        if (!fbi.movementClass.empty())
        {
            movementClassOption = unitDatabase.getMovementClass(fbi.movementClass);
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

        unit.canAttack = fbi.canAttack;

        if (movementClassOption)
        {
            auto movementClass = &movementClassOption->get();

            auto resolvedMovementClass = collisionService->resolveMovementClass(movementClass->name);
            if (!resolvedMovementClass)
            {
                throw std::runtime_error("Failed to resolve movement class " + movementClass->name);
            }

            unit.movementClass = *resolvedMovementClass;
            unit.footprintX = movementClass->footprintX;
            unit.footprintZ = movementClass->footprintZ;
            unit.maxSlope = movementClass->maxSlope;
            unit.maxWaterSlope = movementClass->maxWaterSlope;
            unit.minWaterDepth = movementClass->minWaterDepth;
            unit.maxWaterDepth = movementClass->maxWaterDepth;
        }
        else
        {
            unit.movementClass = std::nullopt;
            unit.footprintX = fbi.footprintX;
            unit.footprintZ = fbi.footprintZ;
            unit.maxSlope = fbi.maxSlope;
            unit.maxWaterSlope = fbi.maxWaterSlope;
            unit.minWaterDepth = fbi.minWaterDepth;
            unit.maxWaterDepth = fbi.maxWaterDepth;
        }

        // add weapons
        if (!fbi.weapon1.empty())
        {
            unit.weapons[0] = createWeapon(fbi.weapon1);
        }
        if (!fbi.weapon2.empty())
        {
            unit.weapons[1] = createWeapon(fbi.weapon2);
        }
        if (!fbi.weapon3.empty())
        {
            unit.weapons[2] = createWeapon(fbi.weapon3);
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

    UnitWeapon UnitFactory::createWeapon(const std::string& weaponType)
    {
        const auto& tdf = unitDatabase.getWeapon(weaponType);
        UnitWeapon weapon;
        weapon.maxRange = tdf.range;
        weapon.reloadTime = tdf.reloadTime;
        weapon.tolerance = toleranceToRadians(tdf.tolerance);
        weapon.pitchTolerance = toleranceToRadians(tdf.pitchTolerance);
        weapon.velocity = static_cast<float>(tdf.weaponVelocity) / 60.0f;
        weapon.duration = tdf.duration * 60.0f * 2.0f; // duration seems to match better if doubled
        if (!tdf.soundStart.empty())
        {
            weapon.soundStart = unitDatabase.getSoundHandle(tdf.soundStart);
        }
        if (!tdf.soundHit.empty())
        {
            weapon.soundHit = unitDatabase.getSoundHandle(tdf.soundHit);
        }
        if (!tdf.soundWater.empty())
        {
            weapon.soundWater = unitDatabase.getSoundHandle(tdf.soundWater);
        }
        return weapon;
    }
}
