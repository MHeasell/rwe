#ifndef RWE_OCCUPIEDGRID_H
#define RWE_OCCUPIEDGRID_H

#include <rwe/FeatureId.h>
#include <rwe/Grid.h>
#include <rwe/UnitId.h>
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

    using OccupiedType = std::variant<OccupiedUnit, OccupiedFeature, OccupiedNone>;

    using OccupiedGrid = Grid<OccupiedType>;
}

#endif
