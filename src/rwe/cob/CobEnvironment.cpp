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

    void CobEnvironment::createThread(unsigned int functionId, const std::vector<int>& params)
    {
        const auto& functionInfo = _script->functions.at(functionId);
        auto& thread = threads.emplace_back(std::make_unique<CobThread>(
            functionInfo.name, functionInfo.address, _script->instructions.size(), params));
        readyQueue.push_back(thread.get());
    }

    void CobEnvironment::createThread(const std::string& functionName, const std::vector<int>& params)
    {
        auto it = std::find_if(_script->functions.begin(), _script->functions.end(), [&functionName](const auto& i) { return i.name == functionName; });
        if (it == _script->functions.end())
        {
            // silently ignore
            return;
        }

        auto index = it - _script->functions.begin();
        createThread(index, params);
    }

    void CobEnvironment::createThread(const std::string& functionName)
    {
        createThread(functionName, std::vector<int>());
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
    }
}
