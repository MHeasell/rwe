#ifndef RWE_OCCUPIEDGRID_H
#define RWE_OCCUPIEDGRID_H

#include "UnitId.h"
#include "Grid.h"
#include <boost/variant.hpp>

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
        bool operator==(const OccupiedFeature&) const { return true; }

        bool operator!=(const OccupiedFeature&) const { return true; }
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
