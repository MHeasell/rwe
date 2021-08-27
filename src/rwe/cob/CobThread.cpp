#include "CobThread.h"

namespace rwe
{
    CobThread::CobThread(const std::string& name, uint32_t signalMask) : name(name), signalMask(signalMask)
    {
    }

    CobThread::CobThread(const std::string& name) : name(name)
    {
    }
}
