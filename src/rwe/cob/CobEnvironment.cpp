#include "CobEnvironment.h"
#include <rwe/GameScene.h>

namespace rwe
{
    void CobEnvironment::showObject(unsigned int objectId)
    {
        const auto& pieceName = _script->pieces.at(objectId);
        scene->showObject(unitId, pieceName);
    }

    void CobEnvironment::hideObject(unsigned int objectId)
    {
        const auto& pieceName = _script->pieces.at(objectId);
        scene->hideObject(unitId, pieceName);
    }

    void CobEnvironment::createThread(unsigned int functionId, const std::vector<int>& params)
    {
        auto& thread = threads.emplace_back(std::make_unique<CobThread>(this));
        thread->setEntryPoint(functionId, params);
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

    CobEnvironment::CobEnvironment(
            GameScene* scene,
            const CobScript* script,
            unsigned int unitId)
            : scene(scene), _script(script), unitId(unitId), _statics(script->staticVariableCount)
    {
    }

    void CobEnvironment::executeThreads()
    {
        while (!readyQueue.empty())
        {
            auto thread = readyQueue.front();
            readyQueue.pop_front();

            thread->execute();

            switch (thread->getStatus())
            {
                case CobThread::Status::Ready:
                    readyQueue.push_back(thread);
                    break;
                case CobThread::Status::Blocked:
                    blockedQueue.push_back(thread);
                    break;
                case CobThread::Status::Finished:
                    deleteThread(thread);
                    break;
            }
        }
    }

    void CobEnvironment::deleteThread(const CobThread* thread)
    {
        auto it = std::find_if(threads.begin(), threads.end(), [thread](const auto& t){ return t.get() == thread; });
        if (it != threads.end())
        {
            threads.erase(it);
        }
    }
}
