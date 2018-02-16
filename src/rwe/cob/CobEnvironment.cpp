#include "CobEnvironment.h"

namespace rwe
{
    CobEnvironment::CobEnvironment(const CobScript* script)
        : _script(script), _statics(script->staticVariableCount)
    {
    }

    int CobEnvironment::getStatic(unsigned int id)
    {
        return _statics.at(id);
    }

    void CobEnvironment::setStatic(unsigned int id, int value)
    {
        _statics.at(id) = value;
    }

    const CobScript* CobEnvironment::script()
    {
        return _script;
    }

    const CobThread* CobEnvironment::createThread(unsigned int functionId, const std::vector<int>& params)
    {
        const auto& functionInfo = _script->functions.at(functionId);
        auto& thread = threads.emplace_back(std::make_unique<CobThread>(
            functionInfo.name, functionInfo.address, _script->instructions.size(), params));
        readyQueue.push_back(thread.get());
        return thread.get();
    }

    boost::optional<const CobThread*> CobEnvironment::createThread(const std::string& functionName, const std::vector<int>& params)
    {
        auto it = std::find_if(_script->functions.begin(), _script->functions.end(), [&functionName](const auto& i) { return i.name == functionName; });
        if (it == _script->functions.end())
        {
            // silently ignore
            return boost::none;
        }

        auto index = it - _script->functions.begin();
        return createThread(index, params);
    }

    boost::optional<const CobThread*> CobEnvironment::createThread(const std::string& functionName)
    {
        return createThread(functionName, std::vector<int>());
    }

    void CobEnvironment::deleteThread(const CobThread* thread)
    {
        auto it = std::find_if(threads.begin(), threads.end(), [thread](const auto& t) { return t.get() == thread; });
        if (it != threads.end())
        {
            threads.erase(it);
        }
    }

    void CobEnvironment::sendSignal(unsigned int signal)
    {
        for (auto it = threads.begin(); it != threads.end();)
        {
            if ((*it)->signalMask & signal)
            {
                // remove references to the thread
                removeThreadFromQueues(it->get());

                // delete the thread
                it = threads.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    boost::optional<int> CobEnvironment::tryReapThread(const CobThread* thread)
    {
        auto it = std::find(finishedQueue.begin(), finishedQueue.end(), thread);
        if (it == finishedQueue.end())
        {
            return boost::none;
        }

        auto val = (*it)->returnValue;
        return val;
    }

    void CobEnvironment::removeThreadFromQueues(const CobThread* thread)
    {
        {
            auto it = std::find(readyQueue.begin(), readyQueue.end(), thread);
            if (it != readyQueue.end())
            {
                readyQueue.erase(it);
            }
        }

        {
            auto it = std::find_if(blockedQueue.begin(), blockedQueue.end(), [thread](const auto& pair) { return pair.second == thread; });
            if (it != blockedQueue.end())
            {
                blockedQueue.erase(it);
            }
        }

        {
            auto it = std::find(finishedQueue.begin(), finishedQueue.end(), thread);
            if (it != finishedQueue.end())
            {
                finishedQueue.erase(it);
            }
        }
    }

    bool CobEnvironment::isNotCorrupt() const
    {
        auto sizes = readyQueue.size() + blockedQueue.size() + finishedQueue.size();
        if (sizes != threads.size())
        {
            return false;
        }

        for (const auto& ptr : threads)
        {
            if (!isPresentInAQueue(ptr.get()))
            {
                return false;
            }
        }

        return true;
    }

    bool CobEnvironment::isPresentInAQueue(const CobThread* thread) const
    {
        {
            auto it = std::find(readyQueue.begin(), readyQueue.end(), thread);
            if (it != readyQueue.end())
            {
                return true;
            }
        }

        {
            auto it = std::find_if(blockedQueue.begin(), blockedQueue.end(), [thread](const auto& pair) { return pair.second == thread; });
            if (it != blockedQueue.end())
            {
                return true;
            }
        }

        {
            auto it = std::find(finishedQueue.begin(), finishedQueue.end(), thread);
            if (it != finishedQueue.end())
            {
                return true;
            }
        }

        return false;
    }
}
