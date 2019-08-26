#pragma once

#include <optional>
#include <string>
#include <utility>

namespace rwe
{
    std::optional<std::pair<std::string, std::string>> getHostAndPort(const std::string& input);
}
