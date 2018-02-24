#include "CobFunction.h"

namespace rwe
{
    CobFunction::CobFunction(unsigned int instructionIndex, const std::vector<int>& locals)
        : instructionIndex(instructionIndex), locals(locals)
    {
    }

    CobFunction::CobFunction(unsigned int instructionIndex) : instructionIndex(instructionIndex)
    {
    }
}
