#include "MovementClass.h"

namespace rwe
{
    MovementClass parseMovementClass(const TdfBlock& block)
    {
        MovementClass m;
        m.name = block.expectString("Name");
        m.footprintX = block.expectUint("FootprintX");
        m.footprintZ = block.expectUint("FootprintZ");
        m.maxWaterDepth = block.extractUint("MinWaterDepth").get_value_or(0);
        m.maxWaterDepth = block.extractUint("MaxWaterDepth").get_value_or(255);
        m.maxSlope = block.extractUint("MaxSlope").get_value_or(0);
        m.maxWaterSlope = block.extractUint("MaxWaterSlope").get_value_or(m.maxSlope);

        return m;
    }

    std::vector<std::pair<std::string, MovementClass>> parseMovementTdf(const TdfBlock& root)
    {
        std::vector<std::pair<std::string, MovementClass>> vec;
        vec.reserve(root.entries.size());

        for (const auto& e : root.entries)
        {
            const auto& block = boost::get<TdfBlock>(*(e.value));
            vec.emplace_back(e.name, parseMovementClass(block));
        }

        return vec;
    }
}
