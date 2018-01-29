#include "OctileDistance_io.h"

namespace rwe
{
    std::ostream& operator<<(std::ostream& stream, const OctileDistance& d)
    {
        stream << "(s:" << d.straight << ", " << d.diagonal << ")";
        return stream;
    }
}
