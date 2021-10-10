#include "UnitFactory.h"
#include <algorithm>
#include <rwe/Index.h>

namespace rwe
{
    UnitFactory::UnitFactory(
        TextureService* textureService,
        UnitDatabase* unitDatabase,
        MeshService&& meshService,
        MovementClassCollisionService* collisionService,
        const ColorPalette* palette,
        const ColorPalette* guiPalette)
        : textureService(textureService),
          unitDatabase(unitDatabase),
          meshService(std::move(meshService)),
          collisionService(collisionService),
          palette(palette),
          guiPalette(guiPalette)
    {
    }

    std::optional<YardMapCell> parseYardMapCell(char c)
    {
        switch (c)
        {
            case 'c':
                return YardMapCell::GroundGeoPassableWhenOpen;
            case 'C':
                return YardMapCell::WaterPassableWhenOpen;
            case 'f':
                return YardMapCell::GroundNoFeature;
            case 'g':
                return YardMapCell::GroundGeoPassableWhenOpen;
            case 'G':
                return YardMapCell::Geo;
            case 'o':
                return YardMapCell::Ground;
            case 'O':
                return YardMapCell::GroundPassableWhenClosed;
            case 'w':
                return YardMapCell::Water;
            case 'y':
                return YardMapCell::GroundPassable;
            case 'Y':
                return YardMapCell::WaterPassable;
            case '.':
                return YardMapCell::Passable;
            case ' ':
                return std::nullopt;
            case '\r':
                return std::nullopt;
            case '\n':
                return std::nullopt;
            case '\t':
                return std::nullopt;
            default:
                return YardMapCell::Ground;
        }
    }
    std::vector<YardMapCell> parseYardMapCells(const std::string& yardMap)
    {
        std::vector<YardMapCell> cells;
        for (const auto& c : yardMap)
        {
            auto cell = parseYardMapCell(c);
            if (cell)
            {
                cells.push_back(*cell);
            }
        }
        return cells;
    }

    Grid<YardMapCell> parseYardMap(unsigned int width, unsigned int height, const std::string& yardMap)
    {
        auto cells = parseYardMapCells(yardMap);
        cells.resize(width * height, YardMapCell::Ground);
        return Grid<YardMapCell>(width, height, std::move(cells));
    }

    std::vector<UnitMesh> createUnitMeshes(const UnitDatabase& db, const std::string& objectName)
    {
        auto def = db.getUnitModelDefinition(objectName);
        if (!def)
        {
            throw std::runtime_error("No definition for object");
        }

        const auto& pieceDefs = def->get().pieces;

        std::vector<UnitMesh> pieces(pieceDefs.size());
        for (Index i = 0; i < getSize(pieces); ++i)
        {
            pieces[i].name = pieceDefs[i].name;
        }

        return pieces;
    }

    Unit UnitFactory::createUnit(
        const std::string& unitType,
        PlayerId owner,
        const SimVector& position,
        std::optional<const std::reference_wrapper<SimAngle>> rotation)
    {
        const auto& fbi = unitDatabase->getUnitInfo(unitType);
        std::optional<std::reference_wrapper<const MovementClass>> movementClassOption;
        if (!fbi.movementClass.empty())
        {
            movementClassOption = unitDatabase->getMovementClass(fbi.movementClass);
        }

        auto meshes = createUnitMeshes(*unitDatabase, fbi.objectName);
        auto modelDefinition = unitDatabase->getUnitModelDefinition(fbi.objectName);
        if (!modelDefinition)
        {
            throw std::runtime_error("Missing model definition");
        }

        if (fbi.bmCode) // unit is mobile
        {
            // don't shade mobile units
            for (auto& m : meshes)
            {
                m.shaded = false;
            }
        }

        const auto& script = unitDatabase->getUnitScript(fbi.unitName);
        auto cobEnv = std::make_unique<CobEnvironment>(&script);
        Unit unit(meshes, std::move(cobEnv));
        unit.name = fbi.name;
        unit.unitType = toUpper(unitType);
        unit.objectName = fbi.objectName;
        unit.owner = owner;
        unit.position = position;
        unit.previousPosition = position;
        unit.height = modelDefinition->get().height;

        if (rotation)
        {
            unit.rotation = *rotation;
            unit.previousRotation = *rotation;
        }
        else if (fbi.bmCode) // unit is mobile
        {
            // spawn the unit facing the other way
            unit.rotation = HalfTurn;
            unit.previousRotation = HalfTurn;
        }

        // These units are per-tick.
        unit.turnRate = toWorldAnglePerTick(fbi.turnRate);
        unit.maxSpeed = SimScalar(fbi.maxVelocity.value);
        unit.acceleration = SimScalar(fbi.acceleration.value);
        unit.brakeRate = SimScalar(fbi.brakeRate.value);

        unit.canAttack = fbi.canAttack;
        unit.canMove = fbi.canMove;
        unit.canGuard = fbi.canGuard;

        unit.commander = fbi.commander;

        unit.maxHitPoints = fbi.maxDamage;

        unit.builder = fbi.builder;

        // Build time is per second, assuming 30 ticks per second.
        unit.buildTime = fbi.buildTime;
        unit.energyCost = fbi.buildCostEnergy;
        unit.metalCost = fbi.buildCostMetal;

        // Worker time is per second, assuming 30 ticks per second.
        unit.workerTimePerTick = fbi.workerTime / 30;

        unit.buildDistance = SimScalar(fbi.buildDistance);

        unit.onOffable = fbi.onOffable;
        unit.activateWhenBuilt = fbi.activateWhenBuilt;

        unit.energyMake = fbi.energyMake;
        unit.energyUse = fbi.energyUse;

        unit.metalMake = fbi.metalMake;
        unit.metalUse = fbi.metalUse - fbi.makesMetal;

        unit.extractsMetal = fbi.extractsMetal;

        unit.energyStorage = fbi.energyStorage;
        unit.metalStorage = fbi.metalStorage;

        unit.hideDamage = fbi.hideDamage;
        unit.showPlayerName = fbi.showPlayerName;

        unit.isMobile = fbi.bmCode;

        unit.floater = fbi.floater;
        unit.canHover = fbi.canHover;

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

        unit.yardMap = parseYardMap(unit.footprintX, unit.footprintZ, fbi.yardMap);
        if (unit.yardMap->any(isWater))
        {
            unit.floater = true;
        }

        // add weapons
        if (!fbi.weapon1.empty())
        {
            unit.weapons[0] = tryCreateWeapon(fbi.weapon1);
        }
        if (!fbi.weapon2.empty())
        {
            unit.weapons[1] = tryCreateWeapon(fbi.weapon2);
        }
        if (!fbi.weapon3.empty())
        {
            unit.weapons[2] = tryCreateWeapon(fbi.weapon3);
        }

        if (!fbi.explodeAs.empty())
        {
            unit.explosionWeapon = tryCreateWeapon(fbi.explodeAs);
        }

        return unit;
    }

    std::optional<std::reference_wrapper<const std::vector<GuiEntry>>> UnitFactory::getBuilderGui(const std::string& unitType, unsigned int page) const
    {
        const auto& pages = unitDatabase->tryGetBuilderGui(unitType);
        if (!pages)
        {
            return std::nullopt;
        }

        const auto& unwrappedPages = pages->get();

        if (page >= unwrappedPages.size())
        {
            return std::nullopt;
        }

        return unwrappedPages[page];
    }

    unsigned int UnitFactory::getBuildPageCount(const std::string& unitType) const
    {
        const auto& pages = unitDatabase->tryGetBuilderGui(unitType);
        if (!pages)
        {
            return 0;
        }

        return pages->get().size();
    }

    Point UnitFactory::getUnitFootprint(const std::string& unitType) const
    {
        const auto& fbi = unitDatabase->getUnitInfo(unitType);
        return Point(fbi.footprintX, fbi.footprintZ);
    }

    MovementClass UnitFactory::getAdHocMovementClass(const std::string& unitType) const
    {
        const auto& fbi = unitDatabase->getUnitInfo(unitType);

        MovementClass mc;
        mc.minWaterDepth = fbi.minWaterDepth;
        mc.maxWaterDepth = fbi.maxWaterDepth;
        mc.maxSlope = fbi.maxSlope;
        mc.maxWaterSlope = fbi.maxWaterSlope;
        mc.footprintX = fbi.footprintX;
        mc.footprintZ = fbi.footprintZ;
        return mc;
    }

    bool UnitFactory::isValidUnitType(const std::string& unitType) const
    {
        return unitDatabase->hasUnitInfo(unitType);
    }

    std::optional<UnitWeapon> UnitFactory::tryCreateWeapon(const std::string& weaponType)
    {
        if (!unitDatabase->tryGetWeapon(toUpper(weaponType)).has_value())
        {
            return std::nullopt;
        }

        UnitWeapon weapon;
        weapon.weaponType = toUpper(weaponType);
        return weapon;
    }
}
