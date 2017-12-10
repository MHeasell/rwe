#include "CobThread.h"

namespace rwe
{
    CobThread::CobThread(
        const std::string& name,
        unsigned int instructionIndex,
        unsigned int endIndex,
        const std::vector<int>& params)
        : name(name),
          instructionIndex(instructionIndex)
    {
        stack.push(endIndex);
        locals.push(params);
    }
}
