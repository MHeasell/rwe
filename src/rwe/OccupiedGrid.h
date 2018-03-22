#ifndef RWE_OCCUPIEDGRID_H
#define RWE_OCCUPIEDGRID_H

#include <boost/variant.hpp>
#include <rwe/FeatureId.h>
#include <rwe/Grid.h>
#include <rwe/UnitId.h>

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

    using OccupiedType = boost::variant<OccupiedUnit, OccupiedFeature, OccupiedNone>;

    struct OccupiedGrid
    {
        Grid<OccupiedType> grid;

        OccupiedGrid(std::size_t width, std::size_t height);
    };
}

#endif
