#include "CobExecutionService.h"
#include <rwe/GameScene.h>
#include <rwe/cob/CobExecutionContext.h>
#include <rwe/overloaded.h>

namespace rwe
{
    Axis toAxis(CobAxis axis)
    {
        switch (axis)
        {
            case CobAxis::X:
                return Axis::X;
            case CobAxis::Y:
                return Axis::Y;
            case CobAxis::Z:
                return Axis::Z;
            default:
                throw std::logic_error("Invalid CobAxis value");
        }
    }

    const std::string& getObjectName(const CobEnvironment& env, unsigned int objectId)
    {
        return env._script->pieces.at(objectId);
    }

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
                    return !simulation.isPieceMoving(unitId, pieceName, toAxis(condition.axis));
                },
                [&env, &simulation, unitId](const CobEnvironment::BlockedStatus::Turn& condition) {
                    const auto& pieceName = env._script->pieces.at(condition.object);
                    return !simulation.isPieceTurning(unitId, pieceName, toAxis(condition.axis));
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
                },
                [&](const CobEnvironment::MotionCommandStatus& status) {
                    env.readyQueue.emplace_front(thread);

                    const auto& objectName = getObjectName(env, status.piece);
                    auto axis = toAxis(status.axis);
                    match(
                        status.command,
                        [&](const CobEnvironment::MotionCommandStatus::Move& m) {
                            // flip x-axis translations to match our right-handed coordinates
                            auto position = status.axis == CobAxis::X ? -m.position : m.position;
                            if (m.speed)
                            {
                                simulation.moveObject(unitId, objectName, axis, position.toWorldDistance(), m.speed->toSimScalar());
                            }
                            else
                            {
                                simulation.moveObjectNow(unitId, objectName, axis, position.toWorldDistance());
                            }
                        },
                        [&](const CobEnvironment::MotionCommandStatus::Turn& t) {
                            // flip z-axis rotations to match our right-handed coordinates
                            auto angle = status.axis == CobAxis::Z ? -t.angle : t.angle;
                            if (t.speed)
                            {
                                simulation.turnObject(unitId, objectName, axis, toWorldAngle(angle), t.speed->toSimScalar());
                            }
                            else
                            {
                                simulation.turnObjectNow(unitId, objectName, axis, toWorldAngle(angle));
                            }
                        },
                        [&](const CobEnvironment::MotionCommandStatus::Spin& s) {
                            simulation.spinObject(unitId, objectName, axis, s.targetSpeed.toSimScalar(), s.acceleration.toSimScalar());
                        },
                        [&](const CobEnvironment::MotionCommandStatus::StopSpin& s) {
                            simulation.stopSpinObject(unitId, objectName, axis, s.deceleration.toSimScalar());
                        });
                });
        }

        assert(env.isNotCorrupt());
    }
}
