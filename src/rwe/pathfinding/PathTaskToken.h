#ifndef RWE_PATHTASKTOKEN_H
#define RWE_PATHTASKTOKEN_H

#include <future>
#include "PathTaskId.h"
#include "UnitPath.h"

namespace rwe
{
    struct PathTaskToken
    {
        PathTaskId id;
        std::future<UnitPath> result;
    };
}

#endif
