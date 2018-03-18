#include "UnitFactory.h"
#include <algorithm>

namespace rwe
{
    UnitFactory::UnitFactory(
        TextureService* textureService,
        UnitDatabase&& unitDatabase,
        MeshService&& meshService,
        MovementClassCollisionService* collisionService,
        const ColorPalette* palette,
        const ColorPalette* guiPalette)
        : textureService(textureService),
          unitDatabase(std::move(unitDatabase)),
          meshService(std::move(meshService)),
          collisionService(collisionService),
          palette(palette),
          guiPalette(guiPalette)
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
        weapon.color = getLaserColor(tdf.color);
        weapon.color2 = getLaserColor(tdf.color2);
        weapon.commandFire = tdf.commandFire;
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
        if (!tdf.explosionGaf.empty() && !tdf.explosionArt.empty())
        {
            weapon.explosion = textureService->getGafEntry("anims/" + tdf.explosionGaf + ".gaf", tdf.explosionArt);
        }
        if (!tdf.waterExplosionGaf.empty() && !tdf.waterExplosionArt.empty())
        {
            weapon.waterExplosion = textureService->getGafEntry("anims/" + tdf.waterExplosionGaf + ".gaf", tdf.waterExplosionArt);
        }
        return weapon;
    }

    Vector3f colorToVector(const Color& color)
    {
        return Vector3f(
            static_cast<float>(color.r) / 255.0f,
            static_cast<float>(color.g) / 255.0f,
            static_cast<float>(color.b) / 255.0f);
    }

    unsigned int colorDistance(const Color& a, const Color& b)
    {
        auto dr = a.r > b.r ? a.r - b.r : b.r - a.r;
        auto dg = a.g > b.g ? a.g - b.g : b.g - a.g;
        auto db = a.b > b.b ? a.b - b.b : b.b - a.b;
        return dr + dg + db;
    }

    Vector3f UnitFactory::getLaserColor(unsigned int colorIndex)
    {
        // In TA, lasers use the GUIPAL colors,
        // but these must be mapped to a color available
        // in the in-game PALETTE.
        const auto& guiColor = (*guiPalette).at(colorIndex);

        auto elem = std::min_element(
            palette->begin(),
            palette->end(),
            [&guiColor](const auto& a, const auto& b) { return colorDistance(guiColor, a) < colorDistance(guiColor, b); });

        return colorToVector(*elem);
    }
}
