#pragma once

#include <vector>

namespace rwe
{
    class CobFunction
    {
    public:
        int instructionIndex;
        std::vector<int> locals;
        int localCount{0};

    public:
        CobFunction(int instructionIndex, const std::vector<int>& locals);

        explicit CobFunction(int instructionIndex);
    };
}
