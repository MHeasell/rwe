#include "OccupiedGrid.h"
#include <rwe/FeatureId.h>

namespace rwe
{
    OccupiedUnit::OccupiedUnit(const UnitId& id) : id(id)
    {
    }

    bool OccupiedUnit::operator==(const OccupiedUnit& rhs) const
    {
        return id == rhs.id;
    }

    bool OccupiedUnit::operator!=(const OccupiedUnit& rhs) const
    {
        return !(rhs == *this);
    }

    OccupiedGrid::OccupiedGrid(std::size_t width, std::size_t height) : grid(width, height, OccupiedType(OccupiedNone())) {}

    OccupiedFeature::OccupiedFeature(const FeatureId& id) : id(id)
    {
    }

    bool OccupiedFeature::operator==(const OccupiedFeature& rhs) const
    {
        return id == rhs.id;
    }

    bool OccupiedFeature::operator!=(const OccupiedFeature& rhs) const
    {
        return !(rhs == *this);
    }
}
