#pragma once

#include <iomanip>
#include <rwe/Energy.h>
#include <rwe/Metal.h>
#include <sstream>
#include <string>

namespace rwe
{
    std::string formatResource(const Metal& v);

    std::string formatResource(const Energy& v);

    std::string formatResourceDelta(const Metal& v);

    std::string formatResourceDelta(const Energy& v);
}
