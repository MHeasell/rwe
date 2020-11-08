#include "UnitFactory.h"
#include <algorithm>

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

    void setShadeRecursive(UnitMesh& mesh, bool shade)
    {
        mesh.shaded = shade;
        for (auto& c : mesh.children)
        {
            setShadeRecursive(c, shade);
        }
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

    UnitMesh toUnitMesh(const UnitDatabase& db, const std::string& unitName, const UnitPieceDefinition& def)
    {
        UnitMesh m;
        m.origin = def.origin;
        m.name = def.name;

        for (const auto& c : def.children)
        {
            m.children.push_back(toUnitMesh(db, unitName, c));
        }

        return m;
    }

    UnitMesh createUnitMesh(const UnitDatabase& db, const std::string& objectName)
    {
        auto def = db.getUnitModelDefinition(objectName);
        if (!def)
        {
            throw std::runtime_error("No definition for object");
        }

        return toUnitMesh(db, objectName, def->get().rootPiece);
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

        auto mesh = createUnitMesh(*unitDatabase, fbi.objectName);
        auto modelDefinition = unitDatabase->getUnitModelDefinition(fbi.objectName);
        if (!modelDefinition)
        {
            throw std::runtime_error("Missing model definition");
        }

        if (fbi.bmCode) // unit is mobile
        {
            // don't shade mobile units
            setShadeRecursive(mesh, false);
        }

        const auto& script = unitDatabase->getUnitScript(fbi.unitName);
        auto cobEnv = std::make_unique<CobEnvironment>(&script);
        Unit unit(mesh, std::move(cobEnv));
        unit.name = fbi.name;
        unit.unitType = toUpper(unitType);
        unit.objectName = fbi.objectName;
        unit.owner = owner;
        unit.position = position;
        unit.height = modelDefinition->get().height;

        if (fbi.bmCode) // unit is mobile
        {
            // spawn the unit facing the other way
            unit.rotation = HalfTurn;
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

        weapon.maxRange = SimScalar(tdf.range);
        weapon.reloadTime = SimScalar(tdf.reloadTime);
        weapon.tolerance = SimAngle(tdf.tolerance);
        weapon.pitchTolerance = SimAngle(tdf.pitchTolerance);
        weapon.velocity = SimScalar(static_cast<float>(tdf.weaponVelocity) / 30.0f);

        weapon.burst = tdf.burst;
        weapon.burstInterval = SimScalar(tdf.burstRate);
        weapon.sprayAngle = SimAngle(tdf.sprayAngle);

        weapon.physicsType = tdf.lineOfSight
            ? ProjectilePhysicsType::LineOfSight
            : tdf.ballistic ? ProjectilePhysicsType::Ballistic
                            : ProjectilePhysicsType::LineOfSight;

        switch (tdf.renderType)
        {
            case 0:
            {
                weapon.renderType = ProjectileRenderTypeLaser{
                    getLaserColor(tdf.color),
                    getLaserColor(tdf.color2),
                    SimScalar(tdf.duration * 30.0f * 2.0f), // duration seems to match better if doubled
                };
                break;
            }
            case 1:
            {
                auto mesh = createUnitMesh(*unitDatabase, tdf.model);
                setShadeRecursive(mesh, false);
                weapon.renderType = ProjectileRenderTypeModel{
                    tdf.model, std::make_shared<UnitMesh>(std::move(mesh)), ProjectileRenderTypeModel::RotationMode::HalfZ};
                break;
            }
            case 3:
            {
                auto mesh = createUnitMesh(*unitDatabase, tdf.model);
                setShadeRecursive(mesh, false);
                weapon.renderType = ProjectileRenderTypeModel{
                    tdf.model, std::make_shared<UnitMesh>(std::move(mesh)), ProjectileRenderTypeModel::RotationMode::QuarterY};
                break;
            }
            case 4:
            {
                weapon.renderType = ProjectileRenderTypeSprite{"fx", getFxName(tdf.color)};
                break;
            }
            case 6:
            {
                auto mesh = createUnitMesh(*unitDatabase, tdf.model);
                setShadeRecursive(mesh, false);
                weapon.renderType = ProjectileRenderTypeModel{
                    tdf.model, std::make_shared<UnitMesh>(std::move(mesh)), ProjectileRenderTypeModel::RotationMode::None};
                break;
            }
            default:
            {
                weapon.renderType = ProjectileRenderTypeLaser{
                    Vector3f(0.0f, 0.0f, 0.0f),
                    Vector3f(0.0f, 0.0f, 0.0f),
                    SimScalar(4.0f)};
                break;
            }
        }
        weapon.commandFire = tdf.commandFire;
        weapon.startSmoke = tdf.startSmoke;
        weapon.endSmoke = tdf.endSmoke;
        if (tdf.smokeTrail)
        {
            weapon.smokeTrail = GameTime(static_cast<unsigned int>(tdf.smokeDelay * 30.0f));
        }

        for (const auto& p : tdf.damage)
        {
            weapon.damage.insert_or_assign(toUpper(p.first), p.second);
        }

        weapon.damageRadius = SimScalar(static_cast<float>(tdf.areaOfEffect) / 2.0f);

        if (tdf.weaponTimer != 0.0f)
        {
            weapon.weaponTimer = GameTime(static_cast<unsigned int>(tdf.weaponTimer * 30.0f));
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
