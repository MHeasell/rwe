#ifndef RWE_OCTILEDISTANCE_IO_H
#define RWE_OCTILEDISTANCE_IO_H

#include <ostream>
#include <rwe/pathfinding/OctileDistance.h>

namespace rwe
{
    std::ostream& operator<<(std::ostream& stream, const OctileDistance& d);
}

#endif
