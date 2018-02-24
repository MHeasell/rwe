#ifndef RWE_COBFUNCTION_H
#define RWE_COBFUNCTION_H

#include <vector>

namespace rwe
{
    class CobFunction
    {
    public:
        unsigned int instructionIndex;
        std::vector<int> locals;

    public:
        CobFunction(unsigned int instructionIndex, const std::vector<int>& locals);

        explicit CobFunction(unsigned int instructionIndex);
    };
}

#endif
