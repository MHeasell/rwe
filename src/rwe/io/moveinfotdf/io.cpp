#include "io.h"

namespace rwe
{
    MovementClassTdf parseMovementClassBlock(const TdfBlock& block)
    {
        MovementClassTdf m;
        m.name = block.expectString("Name");
        m.footprintX = block.expectUint("FootprintX");
        m.footprintZ = block.expectUint("FootprintZ");
        m.minWaterDepth = block.extractUint("MinWaterDepth").value_or(0);
        m.maxWaterDepth = block.extractUint("MaxWaterDepth").value_or(255);
        m.maxSlope = block.extractUint("MaxSlope").value_or(255);
        m.maxWaterSlope = block.extractUint("MaxWaterSlope").value_or(m.maxSlope);

        return m;
    }

    std::vector<std::pair<std::string, MovementClassTdf>> parseMoveInfoTdf(const TdfBlock& root)
    {
        std::vector<std::pair<std::string, MovementClassTdf>> vec;
        vec.reserve(root.blocks.size());

        for (const auto& e : root.blocks)
        {
            vec.emplace_back(e.first, parseMovementClassBlock(*e.second));
        }

        return vec;
    }
}
