#include "CobThread.h"

namespace rwe
{
    CobThread::CobThread(const std::string& name, unsigned int signalMask) : name(name), signalMask(signalMask)
    {
    }

    CobThread::CobThread(const std::string& name) : name(name)
    {
    }
}
