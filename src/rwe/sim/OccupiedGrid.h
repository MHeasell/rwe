#pragma once

#include <rwe/grid/Grid.h>
#include <rwe/sim/FeatureId.h>
#include <rwe/sim/UnitId.h>
#include <variant>

namespace rwe
{
    struct OccupiedUnit
    {
        UnitId id;

        explicit OccupiedUnit(const UnitId& id);

        bool operator==(const OccupiedUnit& rhs) const;

        bool operator!=(const OccupiedUnit& rhs) const;
    };

    struct OccupiedFeature
    {
        FeatureId id;

        explicit OccupiedFeature(const FeatureId& id);

        bool operator==(const OccupiedFeature& rhs) const;

        bool operator!=(const OccupiedFeature& rhs) const;
    };

    struct OccupiedNone
    {
        bool operator==(const OccupiedNone&) const { return true; }

        bool operator!=(const OccupiedNone&) const { return true; }
    };

    using OccupiedType = std::variant<OccupiedNone, OccupiedUnit, OccupiedFeature>;

    struct BuildingOccupiedCell
    {
        UnitId unit;
        bool passable;
    };

    struct OccupiedCell
    {
        OccupiedType occupiedType;
        std::optional<BuildingOccupiedCell> buildingCell;
    };

    using OccupiedGrid = Grid<OccupiedCell>;
}
