#ifndef RWE_PATHTASKTOKEN_H
#define RWE_PATHTASKTOKEN_H

#include "PathTaskId.h"
#include "UnitPath.h"
#include <future>

namespace rwe
{
    struct PathTaskToken
    {
        PathTaskId id;
        std::future<UnitPath> result;
    };
}

#endif
