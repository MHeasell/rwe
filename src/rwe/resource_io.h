#pragma once

#include <iomanip>
#include <rwe/sim/Energy.h>
#include <rwe/sim/Metal.h>
#include <sstream>
#include <string>

namespace rwe
{
    std::string formatResource(const Metal& v);

    std::string formatResource(const Energy& v);

    std::string formatResourceDelta(const Metal& v);

    std::string formatResourceDelta(const Energy& v);
}
