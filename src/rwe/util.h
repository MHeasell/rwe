#pragma once

#include <filesystem>
#include <optional>

namespace rwe
{
    std::optional<std::filesystem::path> getLocalDataPath();
    std::optional<std::filesystem::path> getSearchPath();

    float toleranceToRadians(unsigned int angle);
}
