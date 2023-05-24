#pragma once

#include <rwe/io/moveinfotdf/MovementClassTdf.h>
#include <rwe/io/tdf/TdfBlock.h>
#include <string>
#include <utility>
#include <vector>

namespace rwe
{
    MovementClassTdf parseMovementClassBlock(const TdfBlock& block);

    std::vector<std::pair<std::string, MovementClassTdf>> parseMoveInfoTdf(const TdfBlock& root);
}
