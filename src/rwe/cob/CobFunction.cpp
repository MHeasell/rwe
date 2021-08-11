#include "CobFunction.h"

namespace rwe
{
    CobFunction::CobFunction(int instructionIndex, const std::vector<int>& locals)
        : instructionIndex(instructionIndex), locals(locals)
    {
    }

    CobFunction::CobFunction(int instructionIndex) : instructionIndex(instructionIndex)
    {
    }
}
