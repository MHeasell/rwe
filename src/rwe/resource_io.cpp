#include "resource_io.h"

namespace rwe
{
    std::string formatResource(const Metal& v)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(0) << v.value;
        return ss.str();
    }

    std::string formatResource(const Energy& v)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(0) << v.value;
        return ss.str();
    }

    std::string formatResourceDelta(const Metal& v)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << v.value;
        return ss.str();
    }

    std::string formatResourceDelta(const Energy& v)
    {
        return formatResource(v);
    }
}
