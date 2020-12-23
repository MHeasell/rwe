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
        const SimVector& position)
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

        if (fbi.bmCode) // unit is mobile
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

        unit.hideDamage = fbi.hideDamage;
        unit.showPlayerName = fbi.showPlayerName;

        unit.isMobile = fbi.bmCode;

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

    std::string getFxName(unsigned int code)
    {
        switch (code)
        {
            case 0:
                return "cannonshell";
            case 1:
                return "plasmasm";
            case 2:
                return "plasmamd";
            case 3:
                return "ultrashell";
            case 4:
                return "plasmasm";
            default:
                throw std::runtime_error("Unknown weapon sprite code");
        }
    }

    std::optional<UnitWeapon> UnitFactory::tryCreateWeapon(const std::string& weaponType)
    {
        const auto tdf = unitDatabase->tryGetWeapon(weaponType);
        if (!tdf)
        {
            return std::nullopt;
        }
        return createWeapon(weaponType, *tdf);
    }

    UnitWeapon UnitFactory::createWeapon(const std::string& weaponType)
    {
        const auto& tdf = unitDatabase->getWeapon(weaponType);
        return createWeapon(weaponType, tdf);
    }

    UnitWeapon UnitFactory::createWeapon(const std::string& weaponType, const WeaponTdf& tdf)
    {
        UnitWeapon weapon;

        weapon.weaponType = weaponType;

        weapon.weaponDefinition.maxRange = SimScalar(tdf.range);
        weapon.weaponDefinition.reloadTime = SimScalar(tdf.reloadTime);
        weapon.weaponDefinition.tolerance = SimAngle(tdf.tolerance);
        weapon.weaponDefinition.pitchTolerance = SimAngle(tdf.pitchTolerance);
        weapon.weaponDefinition.velocity = SimScalar(static_cast<float>(tdf.weaponVelocity) / 30.0f);

        weapon.weaponDefinition.burst = tdf.burst;
        weapon.weaponDefinition.burstInterval = SimScalar(tdf.burstRate);
        weapon.weaponDefinition.sprayAngle = SimAngle(tdf.sprayAngle);

        weapon.weaponDefinition.physicsType = tdf.lineOfSight
            ? ProjectilePhysicsType::LineOfSight
            : tdf.ballistic ? ProjectilePhysicsType::Ballistic
                            : ProjectilePhysicsType::LineOfSight;

        switch (tdf.renderType)
        {
            case 0:
            {
                weapon.weaponDefinition.renderType = ProjectileRenderTypeLaser{
                    getLaserColor(tdf.color),
                    getLaserColor(tdf.color2),
                    SimScalar(tdf.duration * 30.0f * 2.0f), // duration seems to match better if doubled
                };
                break;
            }
            case 1:
            {
                weapon.weaponDefinition.renderType = ProjectileRenderTypeModel{
                    tdf.model, ProjectileRenderTypeModel::RotationMode::HalfZ};
                break;
            }
            case 3:
            {
                weapon.weaponDefinition.renderType = ProjectileRenderTypeModel{
                    tdf.model, ProjectileRenderTypeModel::RotationMode::QuarterY};
                break;
            }
            case 4:
            {
                weapon.weaponDefinition.renderType = ProjectileRenderTypeSprite{"fx", getFxName(tdf.color)};
                break;
            }
            case 5:
            {
                weapon.weaponDefinition.renderType = ProjectileRenderTypeFlamethrower{};
                break;
            }
            case 6:
            {
                weapon.weaponDefinition.renderType = ProjectileRenderTypeModel{
                    tdf.model, ProjectileRenderTypeModel::RotationMode::None};
                break;
            }
            case 7:
            {
                weapon.weaponDefinition.renderType = ProjectileRenderTypeLightning{
                    getLaserColor(tdf.color),
                    SimScalar(tdf.duration * 30.0f * 2.0f), // duration seems to match better if doubled
                };
                break;
            }
            default:
            {
                weapon.weaponDefinition.renderType = ProjectileRenderTypeLaser{
                    Vector3f(0.0f, 0.0f, 0.0f),
                    Vector3f(0.0f, 0.0f, 0.0f),
                    SimScalar(4.0f)};
                break;
            }
        }
        weapon.weaponDefinition.commandFire = tdf.commandFire;
        weapon.weaponDefinition.startSmoke = tdf.startSmoke;
        weapon.weaponDefinition.endSmoke = tdf.endSmoke;
        if (tdf.smokeTrail)
        {
            weapon.weaponDefinition.smokeTrail = GameTime(static_cast<unsigned int>(tdf.smokeDelay * 30.0f));
        }

        weapon.weaponDefinition.soundTrigger = tdf.soundTrigger;

        for (const auto& p : tdf.damage)
        {
            weapon.weaponDefinition.damage.insert_or_assign(toUpper(p.first), p.second);
        }

        weapon.weaponDefinition.damageRadius = SimScalar(static_cast<float>(tdf.areaOfEffect) / 2.0f);

        if (tdf.weaponTimer != 0.0f)
        {
            weapon.weaponDefinition.weaponTimer = GameTime(static_cast<unsigned int>(tdf.weaponTimer * 30.0f));
        }

        weapon.weaponDefinition.groundBounce = tdf.groundBounce;

        weapon.weaponDefinition.randomDecay = GameTime(static_cast<unsigned int>(tdf.randomDecay * 30.0f));

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
