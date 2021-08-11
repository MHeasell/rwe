#include "CobThread.h"

namespace rwe
{
    CobThread::CobThread(const std::string& name, int signalMask) : name(name), signalMask(signalMask)
    {
    }

    CobThread::CobThread(const std::string& name) : name(name)
    {
    }
}
