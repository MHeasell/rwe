#include "CobExecutionService.h"
#include <rwe/GameScene.h>
#include <rwe/cob/CobExecutionContext.h>
#include <rwe/match.h>

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

        // check if any sleeping threads can be moved into the ready queue
        for (auto it = env.sleepingQueue.begin(); it != env.sleepingQueue.end();)
        {
            const auto& pair = *it;
            const auto& wakeTime = pair.first;
            if (simulation.gameTime >= wakeTime)
            {
                env.readyQueue.push_back(pair.second);
                it = env.sleepingQueue.erase(it);
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
                [&](const CobEnvironment::SleepStatus& status) {
                    env.sleepingQueue.emplace_back(simulation.gameTime + status.duration.toGameTime(), thread);
                },
                [&env, thread](const CobEnvironment::FinishedStatus&) {
                    env.finishedQueue.emplace_back(thread);
                },
                [&env, thread](const CobEnvironment::SignalStatus& status) {
                    env.readyQueue.emplace_front(thread);
                    env.sendSignal(status.signal);
                },
                [&](const CobEnvironment::PieceCommandStatus& status) {
                    env.readyQueue.emplace_front(thread);

                    const auto& objectName = getObjectName(env, status.piece);
                    match(
                        status.command,
                        [&](const CobEnvironment::PieceCommandStatus::Move& m) {
                            // flip x-axis translations to match our right-handed coordinates
                            auto position = m.axis == CobAxis::X ? -m.position : m.position;
                            if (m.speed)
                            {
                                simulation.moveObject(unitId, objectName, toAxis(m.axis), position.toWorldDistance(), m.speed->toSimScalar());
                            }
                            else
                            {
                                simulation.moveObjectNow(unitId, objectName, toAxis(m.axis), position.toWorldDistance());
                            }
                        },
                        [&](const CobEnvironment::PieceCommandStatus::Turn& t) {
                            // flip z-axis rotations to match our right-handed coordinates
                            auto angle = t.axis == CobAxis::Z ? -t.angle : t.angle;
                            if (t.speed)
                            {
                                simulation.turnObject(unitId, objectName, toAxis(t.axis), toWorldAngle(angle), t.speed->toSimScalar());
                            }
                            else
                            {
                                simulation.turnObjectNow(unitId, objectName, toAxis(t.axis), toWorldAngle(angle));
                            }
                        },
                        [&](const CobEnvironment::PieceCommandStatus::Spin& s) {
                            simulation.spinObject(unitId, objectName, toAxis(s.axis), s.targetSpeed.toSimScalar(), s.acceleration.toSimScalar());
                        },
                        [&](const CobEnvironment::PieceCommandStatus::StopSpin& s) {
                            simulation.stopSpinObject(unitId, objectName, toAxis(s.axis), s.deceleration.toSimScalar());
                        },
                        [&](const CobEnvironment::PieceCommandStatus::Show&) {
                            simulation.showObject(unitId, objectName);
                        },
                        [&](const CobEnvironment::PieceCommandStatus::Hide&) {
                            simulation.hideObject(unitId, objectName);
                        },
                        [&](const CobEnvironment::PieceCommandStatus::EnableShading&) {
                            simulation.enableShading(unitId, objectName);
                        },
                        [&](const CobEnvironment::PieceCommandStatus::DisableShading&) {
                            simulation.disableShading(unitId, objectName);
                        });
                });
        }

        assert(env.isNotCorrupt());
    }
}
