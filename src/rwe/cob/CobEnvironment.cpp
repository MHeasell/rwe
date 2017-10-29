#include "CobEnvironment.h"
#include <rwe/GameScene.h>

namespace rwe
{
    bool CobEnvironment::BlockCheckVisitor::operator()(const CobThread::BlockedStatus::Move& condition) const
    {
        const auto& pieceName = env->_script->pieces.at(condition.object);
        return !env->scene->isPieceMoving(env->unitId, pieceName, condition.axis);
    }

    bool CobEnvironment::BlockCheckVisitor::operator()(const CobThread::BlockedStatus::Turn& condition) const
    {
        const auto& pieceName = env->_script->pieces.at(condition.object);
        return !env->scene->isPieceTurning(env->unitId, pieceName, condition.axis);
    }

    bool CobEnvironment::BlockCheckVisitor::operator()(const CobThread::BlockedStatus::Sleep& condition) const
    {
        return env->getGameTime() >= condition.wakeUpTime;
    }

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
        // check if any blocked threads can be unblocked
        std::vector<CobThread*> tempQueue;
        for (const auto& t : blockedQueue)
        {
            const auto& status = boost::get<CobThread::BlockedStatus>(t->getStatus());

            auto isUnblocked = boost::apply_visitor(BlockCheckVisitor(this), status.condition);
            if (isUnblocked)
            {
                tempQueue.push_back(t);
            }
        }

        // move unblocked threads back into the ready queue
        for (const auto& t : tempQueue)
        {
            auto it = std::find(blockedQueue.begin(), blockedQueue.end(), t);
            blockedQueue.erase(it);
            readyQueue.push_back(t);
            t->setReady();
        }

        // execute ready threads
        while (!readyQueue.empty())
        {
            auto thread = readyQueue.front();
            readyQueue.pop_front();

            thread->execute();

            boost::apply_visitor(ThreadRescheduleVisitor(this, thread),  thread->getStatus());
        }
    }

    void CobEnvironment::deleteThread(const CobThread* thread)
    {
        auto it = std::find_if(threads.begin(), threads.end(), [thread](const auto& t) { return t.get() == thread; });
        if (it != threads.end())
        {
            threads.erase(it);
        }
    }

    void CobEnvironment::moveObject(unsigned int objectId, Axis axis, float position, float speed)
    {
        const auto& pieceName = _script->pieces.at(objectId);
        scene->moveObject(unitId, pieceName, axis, position, speed);
    }

    void CobEnvironment::moveObjectNow(unsigned int objectId, Axis axis, float position)
    {
        const auto& pieceName = _script->pieces.at(objectId);
        scene->moveObjectNow(unitId, pieceName, axis, position);
    }

    void CobEnvironment::turnObject(unsigned int objectId, Axis axis, float angle, float speed)
    {
        const auto& pieceName = _script->pieces.at(objectId);
        scene->turnObject(unitId, pieceName, axis, angle, speed);
    }

    void CobEnvironment::turnObjectNow(unsigned int objectId, Axis axis, float angle)
    {
        const auto& pieceName = _script->pieces.at(objectId);
        scene->turnObjectNow(unitId, pieceName, axis, angle);
    }

    unsigned int CobEnvironment::getGameTime() const
    {
        return scene->getGameTime();
    }
}
