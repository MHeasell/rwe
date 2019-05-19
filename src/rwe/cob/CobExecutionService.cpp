#include "CobExecutionService.h"
#include <rwe/GameScene.h>
#include <rwe/cob/CobExecutionContext.h>
#include <rwe/overloaded.h>

namespace rwe
{
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

            auto isUnblocked = match(
                status.condition,
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
                });

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

            match(
                context.execute(),
                [&env, thread](const CobEnvironment::BlockedStatus& status) {
                    env.blockedQueue.emplace_back(status, thread);
                },
                [&env, thread](const CobEnvironment::FinishedStatus&) {
                    env.finishedQueue.emplace_back(thread);
                },
                [&env, thread](const CobEnvironment::SignalStatus& status) {
                    env.readyQueue.emplace_front(thread);
                    env.sendSignal(status.signal);
                });
        }

        assert(env.isNotCorrupt());
    }
}
