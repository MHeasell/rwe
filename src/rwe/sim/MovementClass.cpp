#include "MovementClass.h"

namespace rwe
{
    MovementClass parseMovementClass(const TdfBlock& block)
    {
        MovementClass m;
        m.name = block.expectString("Name");
        m.footprintX = block.expectUint("FootprintX");
        m.footprintZ = block.expectUint("FootprintZ");
        m.minWaterDepth = block.extractUint("MinWaterDepth").value_or(0);
        m.maxWaterDepth = block.extractUint("MaxWaterDepth").value_or(255);
        m.maxSlope = block.extractUint("MaxSlope").value_or(255);
        m.maxWaterSlope = block.extractUint("MaxWaterSlope").value_or(m.maxSlope);

        return m;
    }

    std::vector<std::pair<std::string, MovementClass>> parseMovementTdf(const TdfBlock& root)
    {
        std::vector<std::pair<std::string, MovementClass>> vec;
        vec.reserve(root.blocks.size());

        for (const auto& e : root.blocks)
        {
            vec.emplace_back(e.first, parseMovementClass(*e.second));
        }

        return vec;
    }
}
