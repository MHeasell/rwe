#ifndef RWE_PATHTASKID_H
#define RWE_PATHTASKID_H

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct PathTaskIdTag;
    using PathTaskId = OpaqueId<unsigned int, PathTaskIdTag>;
}

#endif
