#pragma once

#include <rwe/grid/Grid.h>
#include <rwe/sim/FeatureId.h>
#include <rwe/sim/UnitId.h>
#include <variant>

namespace rwe
{
    struct OccupiedCellBuildingInfo
    {
        UnitId unit;
        bool passable;
    };

    struct OccupiedCell
    {
        std::optional<UnitId> mobileUnitId;
        std::optional<OccupiedCellBuildingInfo> buildingInfo;
        std::optional<FeatureId> featureId;
    };

    using OccupiedGrid = Grid<OccupiedCell>;
}
