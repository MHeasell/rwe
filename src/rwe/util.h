#pragma once

#include <boost/filesystem/path.hpp>
#include <optional>

namespace rwe
{
    std::optional<boost::filesystem::path> getLocalDataPath();
    std::optional<boost::filesystem::path> getSearchPath();

    float toleranceToRadians(unsigned int angle);
}
