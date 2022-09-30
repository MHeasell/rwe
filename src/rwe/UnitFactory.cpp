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
        const ColorPalette* guiPalette,
        const GameSimulation* simulation)
        : textureService(textureService),
          unitDatabase(unitDatabase),
          meshService(std::move(meshService)),
          collisionService(collisionService),
          palette(palette),
          guiPalette(guiPalette),
          simulation(simulation)
    {
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

    UnitState UnitFactory::createUnit(
        const std::string& unitType,
        PlayerId owner,
        const SimVector& position,
        std::optional<const std::reference_wrapper<SimAngle>> rotation)
    {
        const auto& unitDefinition = simulation->unitDefinitions.at(unitType);

        auto meshes = createUnitMeshes(*unitDatabase, unitDefinition.objectName);
        auto modelDefinition = unitDatabase->getUnitModelDefinition(unitDefinition.objectName);
        if (!modelDefinition)
        {
            throw std::runtime_error("Missing model definition");
        }

        if (unitDefinition.isMobile)
        {
            // don't shade mobile units
            for (auto& m : meshes)
            {
                m.shaded = false;
            }
        }

        const auto& script = unitDatabase->getUnitScript(unitType);
        auto cobEnv = std::make_unique<CobEnvironment>(&script);
        UnitState unit(meshes, std::move(cobEnv));
        unit.unitType = toUpper(unitType);
        unit.owner = owner;
        unit.position = position;
        unit.previousPosition = position;

        if (rotation)
        {
            unit.rotation = *rotation;
            unit.previousRotation = *rotation;
        }
        else if (unitDefinition.isMobile)
        {
            // spawn the unit facing the other way
            unit.rotation = HalfTurn;
            unit.previousRotation = HalfTurn;
        }

        // add weapons
        if (!unitDefinition.weapon1.empty())
        {
            unit.weapons[0] = tryCreateWeapon(unitDefinition.weapon1);
        }
        if (!unitDefinition.weapon2.empty())
        {
            unit.weapons[1] = tryCreateWeapon(unitDefinition.weapon2);
        }
        if (!unitDefinition.weapon3.empty())
        {
            unit.weapons[2] = tryCreateWeapon(unitDefinition.weapon3);
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
        if (simulation->weaponDefinitions.find(toUpper(weaponType)) == simulation->weaponDefinitions.end())
        {
            return std::nullopt;
        }

        UnitWeapon weapon;
        weapon.weaponType = toUpper(weaponType);
        return weapon;
    }
}
