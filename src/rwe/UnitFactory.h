#pragma once

#include <optional>
#include <rwe/UnitDatabase.h>
#include <rwe/sim/GameSimulation.h>
#include <rwe/sim/MovementClass.h>
#include <rwe/sim/SimVector.h>
#include <rwe/sim/UnitState.h>
#include <string>

namespace rwe
{
    class UnitFactory
    {
    private:
        UnitDatabase* const unitDatabase;
        const GameSimulation* simulation;

    public:
        UnitFactory(
            UnitDatabase* unitDatabase,
            const GameSimulation* simulation);

    public:
        std::optional<std::reference_wrapper<const std::vector<GuiEntry>>> getBuilderGui(const std::string& unitType, unsigned int page) const;

        /** If the unit has no build gui, this will be zero. */
        unsigned int getBuildPageCount(const std::string& unitType) const;

        Point getUnitFootprint(const std::string& unitType) const;

        MovementClass getAdHocMovementClass(const std::string& unitType) const;

        bool isValidUnitType(const std::string& unitType) const;
    };
}
