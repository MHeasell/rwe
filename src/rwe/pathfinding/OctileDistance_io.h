#pragma once

#include <ostream>
#include <rwe/pathfinding/OctileDistance.h>

namespace rwe
{
    std::ostream& operator<<(std::ostream& stream, const OctileDistance& d);
}
