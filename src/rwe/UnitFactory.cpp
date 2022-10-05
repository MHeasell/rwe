#include "UnitFactory.h"
#include <algorithm>

namespace rwe
{
    UnitFactory::UnitFactory(
        UnitDatabase* unitDatabase,
        const GameSimulation* simulation)
        : unitDatabase(unitDatabase),
          simulation(simulation)
    {
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
        const auto& unitDefinition = simulation->unitDefinitions.at(unitType);
        auto [footprintX, footprintZ] = simulation->getFootprintXZ(unitDefinition.movementCollisionInfo);
        return Point(footprintX, footprintZ);
    }

    MovementClass UnitFactory::getAdHocMovementClass(const std::string& unitType) const
    {
        const auto& unitDefinition = simulation->unitDefinitions.at(unitType);
        return simulation->getAdHocMovementClass(unitDefinition.movementCollisionInfo);
    }

    bool UnitFactory::isValidUnitType(const std::string& unitType) const
    {
        return simulation->unitDefinitions.find(unitType) != simulation->unitDefinitions.end();
    }
}
