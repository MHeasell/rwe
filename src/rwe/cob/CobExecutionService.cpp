#include "CobExecutionService.h"
#include <rwe/GameScene.h>
#include <rwe/cob/CobExecutionContext.h>
#include <rwe/cob/cob_util.h>
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

    using InterruptedReason = std::variant<CobEnvironment::PieceCommandStatus, CobEnvironment::QueryStatus, CobEnvironment::SetQueryStatus>;

    std::optional<InterruptedReason> executeThreads(CobEnvironment& env, UnitId unitId, GameTime gameTime)
    {
        while (!env.readyQueue.empty())
        {
            auto thread = env.readyQueue.front();

            CobExecutionContext context(&env, thread, unitId);

            auto result = match(
                context.execute(),
                [&](const CobEnvironment::BlockedStatus& status) {
                    env.readyQueue.pop_front();
                    env.blockedQueue.emplace_back(status, thread);
                    return std::optional<InterruptedReason>();
                },
                [&](const CobEnvironment::SleepStatus& status) {
                    env.readyQueue.pop_front();
                    env.sleepingQueue.emplace_back(gameTime + status.duration.toGameTime(), thread);
                    return std::optional<InterruptedReason>();
                },
                [&](const CobEnvironment::FinishedStatus&) {
                    env.readyQueue.pop_front();
                    env.finishedQueue.emplace_back(thread);
                    return std::optional<InterruptedReason>();
                },
                [&](const CobEnvironment::SignalStatus& status) {
                    env.sendSignal(status.signal);
                    return std::optional<InterruptedReason>();
                },
                [&](const CobEnvironment::PieceCommandStatus& status) {
                    return std::optional<InterruptedReason>(status);
                },
                [&](const CobEnvironment::QueryStatus& status) {
                    return std::optional<InterruptedReason>(status);
                },
                [&](const CobEnvironment::SetQueryStatus& status) {
                    return std::optional<InterruptedReason>(status);
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

    int handleQuery(GameScene& scene, GameSimulation& sim, const CobEnvironment& env, UnitId unitId, const CobEnvironment::QueryStatus& result)
    {
        return match(
            result.query,
            [&](const CobEnvironment::QueryStatus::Random& q) {
                // FIXME: probably not consistent across platforms
                std::uniform_int_distribution<int> dist(q.low, q.high);
                auto value = dist(sim.rng);
                return value;
            },
            [&](const CobEnvironment::QueryStatus::Activation&) {
                const auto& unit = sim.getUnit(unitId);
                return static_cast<int>(unit.activated);
            },
            [&](const CobEnvironment::QueryStatus::StandingFireOrders&) {
                return 0; // TODO
            },
            [&](const CobEnvironment::QueryStatus::StandingMoveOrders&) {
                return 0; // TODO
            },
            [&](const CobEnvironment::QueryStatus::Health&) {
                const auto& unit = sim.getUnit(unitId);
                return static_cast<int>(unit.hitPoints / unit.maxHitPoints);
            },
            [&](const CobEnvironment::QueryStatus::InBuildStance&) {
                const auto& unit = sim.getUnit(unitId);
                return static_cast<int>(unit.inBuildStance);
            },
            [&](const CobEnvironment::QueryStatus::Busy&) {
                return 0;
            },
            [&](const CobEnvironment::QueryStatus::PieceXZ& q) {
                auto pieceId = q.piece;
                const auto& pieceName = getObjectName(env, pieceId);
                const auto& unit = sim.getUnit(unitId);
                auto pieceTransform = unit.mesh.getPieceTransform(pieceName);
                if (!pieceTransform)
                {
                    throw std::runtime_error("Unknown piece " + pieceName);
                }
                auto pos = unit.getTransform() * (*pieceTransform) * SimVector(0_ss, 0_ss, 0_ss);
                return static_cast<int>(packCoords(pos.x, pos.z));
            },
            [&](const CobEnvironment::QueryStatus::PieceY& q) {
                auto pieceId = q.piece;
                const auto& pieceName = getObjectName(env, pieceId);
                const auto& unit = sim.getUnit(unitId);
                auto pieceTransform = unit.mesh.getPieceTransform(pieceName);
                if (!pieceTransform)
                {
                    throw std::runtime_error("Unknown piece " + pieceName);
                }
                const auto& pos = unit.getTransform() * (*pieceTransform) * SimVector(0_ss, 0_ss, 0_ss);
                return simScalarToFixed(pos.y);
            },
            [&](const CobEnvironment::QueryStatus::UnitXZ& q) {
                auto targetUnitId = q.targetUnitId;
                auto targetUnitOption = sim.tryGetUnit(targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: not sure if correct return value when unit does not exist
                    return 0;
                }
                const auto& pos = targetUnitOption->get().position;
                return static_cast<int>(packCoords(pos.x, pos.z));
            },
            [&](const CobEnvironment::QueryStatus::UnitY& q) {
                auto targetUnitId = q.targetUnitId;
                auto targetUnitOption = sim.tryGetUnit(targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: not sure if correct return value when unit does not exist
                    return 0;
                }
                const auto& pos = targetUnitOption->get().position;
                return simScalarToFixed(pos.y);
            },
            [&](const CobEnvironment::QueryStatus::UnitHeight& q) {
                auto targetUnitId = q.targetUnitId;
                auto targetUnitOption = sim.tryGetUnit(targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: not sure if correct return value when unit does not exist
                    return 0;
                }
                return simScalarToFixed(targetUnitOption->get().height);
            },
            [&](const CobEnvironment::QueryStatus::XZAtan& q) {
                auto pair = unpackCoords(q.coords);
                const auto& unit = sim.getUnit(unitId);

                // Surprisingly, the result of XZAtan is offset by the unit's current rotation.
                // The other interesting thing is that in TA, at least for mobile units,
                // it appears that a unit with rotation 0 faces up, towards negative Z.
                // However, in RWE, a unit with rotation 0 faces down, towards positive z.
                // We therefore subtract a half turn to convert to what scripts expect.
                // TODO: test whether this is also the case for buildings
                auto correctedUnitRotation = unit.rotation - HalfTurn;
                auto result = atan2(pair.first, pair.second) - correctedUnitRotation;
                return static_cast<int>(toCobAngle(result).value);
            },
            [&](const CobEnvironment::QueryStatus::GroundHeight& q) {
                auto pair = unpackCoords(q.coords);
                auto result = sim.terrain.getHeightAt(pair.first, pair.second);
                return simScalarToFixed(result);
            },
            [&](const CobEnvironment::QueryStatus::BuildPercentLeft&) {
                const auto& unit = sim.getUnit(unitId);
                return static_cast<int>(unit.getBuildPercentLeft());
            },
            [&](const CobEnvironment::QueryStatus::YardOpen&) {
                const auto& unit = sim.getUnit(unitId);
                return static_cast<int>(unit.yardOpen);
            },
            [&](const CobEnvironment::QueryStatus::BuggerOff&) {
                return 0; // TODO
            },
            [&](const CobEnvironment::QueryStatus::Armored&) {
                return 0; // TODO
            },
            [&](const CobEnvironment::QueryStatus::VeteranLevel&) {
                return 0; // TODO
            },
            [&](const CobEnvironment::QueryStatus::MinId&) {
                return 0; // TODO
            },
            [&](const CobEnvironment::QueryStatus::MaxId&) {
                return static_cast<int>(sim.nextUnitId.value - 1);
            },
            [&](const CobEnvironment::QueryStatus::MyId&) {
                return static_cast<int>(unitId.value);
            },
            [&](const CobEnvironment::QueryStatus::UnitTeam& q) {
                auto targetUnitId = q.targetUnitId;
                auto targetUnitOption = sim.tryGetUnit(targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: unsure if correct return value when unit does not exist
                    return 0;
                }
                // TODO: return player's team instead of player ID
                return static_cast<int>(targetUnitOption->get().owner.value);
            },
            [&](const CobEnvironment::QueryStatus::UnitBuildPercentLeft& q) {
                auto targetUnitOption = sim.tryGetUnit(q.targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: unsure if correct return value when unit does not exist
                    return 0;
                }
                return static_cast<int>(targetUnitOption->get().getBuildPercentLeft());
            },
            [&](const CobEnvironment::QueryStatus::UnitAllied& q) {
                const auto& unit = sim.getUnit(unitId);
                auto targetUnitOption = sim.tryGetUnit(q.targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: unsure if correct return value when unit does not exist
                    return 0;
                }
                // TODO: real allied check including teams/alliances
                return static_cast<int>(targetUnitOption->get().isOwnedBy(unit.owner));
            });
    }

    void handleSetQuery(GameScene& scene, GameSimulation& sim, const CobEnvironment& env, UnitId unitId, const CobEnvironment::SetQueryStatus& result)
    {
        match(
            result.query,
            [&](const CobEnvironment::SetQueryStatus::Activation& q) {
                if (q.value)
                {
                    scene.activateUnit(unitId);
                }
                else
                {
                    scene.deactivateUnit(unitId);
                }
            },
            [&](const CobEnvironment::SetQueryStatus::StandingMoveOrders&) {
                // TODO
            },
            [&](const CobEnvironment::SetQueryStatus::StandingFireOrders&) {
                // TODO
            },
            [&](const CobEnvironment::SetQueryStatus::InBuildStance& q) {
                scene.setBuildStance(unitId, q.value);
            },
            [&](const CobEnvironment::SetQueryStatus::Busy&) {
                // TODO
            },
            [&](const CobEnvironment::SetQueryStatus::YardOpen& q) {
                scene.setYardOpen(unitId, q.value);
            },
            [&](const CobEnvironment::SetQueryStatus::BuggerOff& q) {
                scene.setBuggerOff(unitId, q.value);
            },
            [&](const CobEnvironment::SetQueryStatus::Armored&) {
                // TODO
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
        while (auto result = executeThreads(env, unitId, simulation.gameTime))
        {
            match(
                *result,
                [&](const CobEnvironment::PieceCommandStatus& s) {
                    handlePieceCommand(scene, simulation, env, unitId, s);
                },
                [&](const CobEnvironment::QueryStatus& s) {
                    auto result = handleQuery(scene, simulation, env, unitId, s);
                    env.pushResult(result);
                },
                [&](const CobEnvironment::SetQueryStatus& s) {
                    handleSetQuery(scene, simulation, env, unitId, s);
                });
        }

        assert(env.isNotCorrupt());
    }
}
