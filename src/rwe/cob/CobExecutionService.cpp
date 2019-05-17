#include "CobExecutionService.h"
#include <rwe/GameScene.h>
#include <rwe/cob/CobExecutionContext.h>
#include <rwe/overloaded.h>

namespace rwe
{
    class ThreadRescheduleVisitor
    {
    private:
        CobEnvironment* const env;
        CobThread* const thread;

    public:
        ThreadRescheduleVisitor(CobEnvironment* env, CobThread* thread) : env(env), thread(thread)
        {
        }

        void operator()(const CobEnvironment::BlockedStatus& status) const
        {
            env->blockedQueue.emplace_back(status, thread);
        }
        void operator()(const CobEnvironment::FinishedStatus&) const
        {
            env->finishedQueue.emplace_back(thread);
        }
        void operator()(const CobEnvironment::SignalStatus& status) const
        {
            env->readyQueue.emplace_front(thread);
            env->sendSignal(status.signal);
        }
    };

    void CobExecutionService::run(GameScene& scene, GameSimulation& simulation, UnitId unitId)
    {
        auto& unit = simulation.getUnit(unitId);
        auto& env = *unit.cobEnvironment;

        assert(env.isNotCorrupt());

        // clean up any finished threads that were not reaped last frame
        for (const auto& thread : env.finishedQueue)
        {
            env.deleteThread(thread);
        }
        env.finishedQueue.clear();

        assert(env.isNotCorrupt());

        // check if any blocked threads can be unblocked
        // and move them back into the ready queue
        for (auto it = env.blockedQueue.begin(); it != env.blockedQueue.end();)
        {
            const auto& pair = *it;
            const auto& status = pair.first;

            auto isUnblocked = std::visit(
                overloaded{
                    [&env, &simulation, unitId](const CobEnvironment::BlockedStatus::Move& condition) {
                        const auto& pieceName = env._script->pieces.at(condition.object);
                        return !simulation.isPieceMoving(unitId, pieceName, condition.axis);
                    },
                    [&env, &simulation, unitId](const CobEnvironment::BlockedStatus::Turn& condition) {
                        const auto& pieceName = env._script->pieces.at(condition.object);
                        return !simulation.isPieceTurning(unitId, pieceName, condition.axis);
                    },
                    [&simulation](const CobEnvironment::BlockedStatus::Sleep& condition) {
                        return simulation.gameTime >= condition.wakeUpTime;
                    }},
                status.condition);

            if (isUnblocked)
            {
                env.readyQueue.push_back(pair.second);
                it = env.blockedQueue.erase(it);
            }
            else
            {
                ++it;
            }
        }

        assert(env.isNotCorrupt());

        // execute ready threads
        while (!env.readyQueue.empty())
        {
            auto thread = env.readyQueue.front();
            env.readyQueue.pop_front();

            CobExecutionContext context(&scene, &simulation, &env, thread, unitId);

            auto status = context.execute();

            std::visit(ThreadRescheduleVisitor(&env, thread), status);
        }

        assert(env.isNotCorrupt());
    }
}
