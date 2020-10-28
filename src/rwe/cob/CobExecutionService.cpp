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

    std::optional<CobEnvironment::PieceCommandStatus> executeThreads(GameScene& scene, GameSimulation& simulation, CobEnvironment& env, UnitId unitId)
    {
        while (!env.readyQueue.empty())
        {
            auto thread = env.readyQueue.front();

            CobExecutionContext context(&scene, &simulation, &env, thread, unitId);

            auto result = match(
                context.execute(),
                [&](const CobEnvironment::BlockedStatus& status) {
                    env.readyQueue.pop_front();
                    env.blockedQueue.emplace_back(status, thread);
                    return std::optional<CobEnvironment::PieceCommandStatus>();
                },
                [&](const CobEnvironment::SleepStatus& status) {
                    env.readyQueue.pop_front();
                    env.sleepingQueue.emplace_back(simulation.gameTime + status.duration.toGameTime(), thread);
                    return std::optional<CobEnvironment::PieceCommandStatus>();
                },
                [&](const CobEnvironment::FinishedStatus&) {
                    env.readyQueue.pop_front();
                    env.finishedQueue.emplace_back(thread);
                    return std::optional<CobEnvironment::PieceCommandStatus>();
                },
                [&](const CobEnvironment::SignalStatus& status) {
                    env.sendSignal(status.signal);
                    return std::optional<CobEnvironment::PieceCommandStatus>();
                },
                [&](const CobEnvironment::PieceCommandStatus& status) {
                    return std::optional<CobEnvironment::PieceCommandStatus>(status);
                });

            if (result)
            {
                return result;
            }
        }

        return std::nullopt;
    }

    void handlePieceCommand(GameScene& scene, GameSimulation& simulation, const CobEnvironment& env, UnitId unitId, const CobEnvironment::PieceCommandStatus& result)
    {
        const auto& objectName = getObjectName(env, result.piece);
        match(
            result.command,
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
        while (auto result = executeThreads(scene, simulation, env, unitId))
        {
            handlePieceCommand(scene, simulation, env, unitId, *result);
        }

        assert(env.isNotCorrupt());
    }
}
