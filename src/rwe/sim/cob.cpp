#include "cob.h"
#include <optional>
#include <rwe/cob/CobAxis.h>
#include <rwe/cob/CobExecutionContext.h>
#include <rwe/cob/CobPosition.h>
#include <rwe/cob/CobTime.h>
#include <rwe/cob/cob_util.h>
#include <rwe/sim/SimAxis.h>
#include <rwe/sim/SimScalar.h>

namespace rwe
{
    float toFloat(CobSpeed speed)
    {
        return static_cast<float>(speed.value) / 65536.0f;
    }

    SimScalar toSimScalar(CobSpeed speed)
    {
        return SimScalar(toFloat(speed));
    }

    CobSpeed toCobSpeed(SimScalar speed)
    {
        return CobSpeed(static_cast<uint32_t>(speed.value * 65536.0f));
    }

    SimScalar toSimScalar(CobAngularSpeed angularSpeed)
    {
        return SimScalar(angularSpeed.value);
    }

    SimAngle toWorldAngle(CobAngle angle)
    {
        return SimAngle(angle.value);
    }

    CobAngle toCobAngle(SimAngle angle)
    {
        return CobAngle(angle.value);
    }

    CobTime toCobTime(GameTime gameTime)
    {
        return CobTime((gameTime.value * 1000) / 30);
    }

    CobTime addDuration(CobTime time, CobSleepDuration duration)
    {
        return CobTime(time.value + duration.value);
    }

    SimAxis toSimAxis(CobAxis axis)
    {
        switch (axis)
        {
            case CobAxis::X:
                return SimAxis::X;
            case CobAxis::Y:
                return SimAxis::Y;
            case CobAxis::Z:
                return SimAxis::Z;
            default:
                throw std::logic_error("Invalid CobAxis value");
        }
    }

    const std::string& getObjectName(const CobEnvironment& env, unsigned int objectId)
    {
        return env._script->pieces.at(objectId);
    }

    CobPosition simScalarToCobPosition(SimScalar v)
    {
        return CobPosition(static_cast<int32_t>(v.value * 65536.0f));
    }

    SimScalar cobPositionToSimScalar(CobPosition v)
    {
        return SimScalar(static_cast<float>(v.value) / 65536.0f);
    }

    int packCoords(SimScalar x, SimScalar z)
    {
        return static_cast<int>(cobPackCoords(simScalarToCobPosition(x), simScalarToCobPosition(z)));
    }

    using InterruptedReason = std::variant<CobEnvironment::PieceCommandStatus, CobEnvironment::QueryStatus, CobEnvironment::SetQueryStatus>;

    std::optional<InterruptedReason> executeThreads(CobEnvironment& env, UnitId unitId, GameTime gameTime)
    {
        while (!env.readyQueue.empty())
        {
            auto thread = env.readyQueue.front();

            CobExecutionContext context(&env, thread);

            auto result = match(
                context.execute(),
                [&](const CobEnvironment::BlockedStatus& status) {
                    env.readyQueue.pop_front();
                    env.blockedQueue.emplace_back(status, thread);
                    return std::optional<InterruptedReason>();
                },
                [&](const CobEnvironment::SleepStatus& status) {
                    auto wakeTime = addDuration(toCobTime(gameTime), status.duration);
                    env.readyQueue.pop_front();
                    env.sleepingQueue.emplace_back(wakeTime, thread);
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

    void handlePieceCommand(GameSimulation& simulation, const CobEnvironment& env, UnitId unitId, const CobEnvironment::PieceCommandStatus& result)
    {
        const auto& objectName = getObjectName(env, result.piece);
        match(
            result.command,
            [&](const CobEnvironment::PieceCommandStatus::Move& m) {
                // flip x-axis translations to match our right-handed coordinates
                auto position = m.axis == CobAxis::X ? -m.position : m.position;
                if (m.speed)
                {
                    simulation.moveObject(unitId, objectName, toSimAxis(m.axis), cobPositionToSimScalar(position), toSimScalar(*m.speed));
                }
                else
                {
                    simulation.moveObjectNow(unitId, objectName, toSimAxis(m.axis), cobPositionToSimScalar(position));
                }
            },
            [&](const CobEnvironment::PieceCommandStatus::Turn& t) {
                // flip z-axis rotations to match our right-handed coordinates
                auto angle = t.axis == CobAxis::Z ? -t.angle : t.angle;
                if (t.speed)
                {
                    simulation.turnObject(unitId, objectName, toSimAxis(t.axis), toWorldAngle(angle), toSimScalar(*t.speed));
                }
                else
                {
                    simulation.turnObjectNow(unitId, objectName, toSimAxis(t.axis), toWorldAngle(angle));
                }
            },
            [&](const CobEnvironment::PieceCommandStatus::Spin& s) {
                simulation.spinObject(unitId, objectName, toSimAxis(s.axis), toSimScalar(s.targetSpeed), toSimScalar(s.acceleration));
            },
            [&](const CobEnvironment::PieceCommandStatus::StopSpin& s) {
                simulation.stopSpinObject(unitId, objectName, toSimAxis(s.axis), toSimScalar(s.deceleration));
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
            },
            [&](const CobEnvironment::PieceCommandStatus::EmitSfx& s) {
                switch (s.sfxType)
                {
                    case CobSfxType::WhiteSmoke:
                        simulation.events.push_back(EmitParticleFromPieceEvent{EmitParticleFromPieceEvent::SfxType::LightSmoke, unitId, objectName});
                        break;
                    case CobSfxType::BlackSmoke:
                        simulation.events.push_back(EmitParticleFromPieceEvent{EmitParticleFromPieceEvent::SfxType::BlackSmoke, unitId, objectName});
                        break;
                    case CobSfxType::Wake1:
                        simulation.events.push_back(EmitParticleFromPieceEvent{EmitParticleFromPieceEvent::SfxType::Wake1, unitId, objectName});
                        break;
                    case CobSfxType::Vtol:
                    case CobSfxType::Thrust:
                    case CobSfxType::Wake2:
                    case CobSfxType::ReverseWake1:
                    case CobSfxType::ReverseWake2:
                        // TODO: support these SFX types
                        break;
                }
            });
    }

    int handleQuery(GameSimulation& sim, const CobEnvironment& env, UnitId unitId, const CobEnvironment::QueryStatus& result)
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
                const auto& unit = sim.getUnitState(unitId);
                return static_cast<int>(unit.activated);
            },
            [&](const CobEnvironment::QueryStatus::StandingFireOrders&) {
                return 0; // TODO
            },
            [&](const CobEnvironment::QueryStatus::StandingMoveOrders&) {
                return 0; // TODO
            },
            [&](const CobEnvironment::QueryStatus::Health&) {
                const auto& unit = sim.getUnitState(unitId);
                const auto& unitDefinition = sim.unitDefinitions.at(unit.unitType);
                return static_cast<int>((unit.hitPoints * 100) / unitDefinition.maxHitPoints);
            },
            [&](const CobEnvironment::QueryStatus::InBuildStance&) {
                const auto& unit = sim.getUnitState(unitId);
                return static_cast<int>(unit.inBuildStance);
            },
            [&](const CobEnvironment::QueryStatus::Busy&) {
                return 0;
            },
            [&](const CobEnvironment::QueryStatus::PieceXZ& q) {
                auto pieceId = q.piece;
                const auto& pieceName = getObjectName(env, pieceId);
                auto pos = sim.getUnitPiecePosition(unitId, pieceName);
                return packCoords(pos.x, pos.z);
            },
            [&](const CobEnvironment::QueryStatus::PieceY& q) {
                auto pieceId = q.piece;
                const auto& pieceName = getObjectName(env, pieceId);
                auto pos = sim.getUnitPiecePosition(unitId, pieceName);
                return simScalarToCobPosition(pos.y).value;
            },
            [&](const CobEnvironment::QueryStatus::UnitXZ& q) {
                auto targetUnitId = q.targetUnitId;
                auto targetUnitOption = sim.tryGetUnitState(targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: not sure if correct return value when unit does not exist
                    return 0;
                }
                const auto& pos = targetUnitOption->get().position;
                return packCoords(pos.x, pos.z);
            },
            [&](const CobEnvironment::QueryStatus::UnitY& q) {
                auto targetUnitId = q.targetUnitId;
                auto targetUnitOption = sim.tryGetUnitState(targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: not sure if correct return value when unit does not exist
                    return 0;
                }
                const auto& pos = targetUnitOption->get().position;
                return simScalarToCobPosition(pos.y).value;
            },
            [&](const CobEnvironment::QueryStatus::UnitHeight& q) {
                auto targetUnitId = q.targetUnitId;
                auto targetUnitOption = sim.tryGetUnitState(targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: not sure if correct return value when unit does not exist
                    return 0;
                }
                const auto& unitDefinition = sim.unitDefinitions.at(targetUnitOption->get().unitType);
                const auto& modelDefinition = sim.unitModelDefinitions.at(unitDefinition.objectName);
                return simScalarToCobPosition(modelDefinition.height).value;
            },
            [&](const CobEnvironment::QueryStatus::XZAtan& q) {
                auto pair = cobUnpackCoords(q.coords);
                const auto& unit = sim.getUnitState(unitId);

                // Surprisingly, the result of XZAtan is offset by the unit's current rotation.
                // The other interesting thing is that in TA, at least for mobile units,
                // it appears that a unit with rotation 0 faces up, towards negative Z.
                // However, in RWE, a unit with rotation 0 faces down, towards positive z.
                // We therefore subtract a half turn to convert to what scripts expect.
                // TODO: test whether this is also the case for buildings
                auto correctedUnitRotation = unit.rotation - HalfTurn;
                auto result = atan2(cobPositionToSimScalar(pair.first), cobPositionToSimScalar(pair.second)) - correctedUnitRotation;
                return static_cast<int>(toCobAngle(result).value);
            },
            [&](const CobEnvironment::QueryStatus::GroundHeight& q) {
                auto pair = cobUnpackCoords(q.coords);
                auto result = sim.terrain.getHeightAt(cobPositionToSimScalar(pair.first), cobPositionToSimScalar(pair.second));
                return simScalarToCobPosition(result).value;
            },
            [&](const CobEnvironment::QueryStatus::BuildPercentLeft&) {
                const auto& unit = sim.getUnitState(unitId);
                const auto& unitDefinition = sim.unitDefinitions.at(unit.unitType);
                return static_cast<int>(unit.getBuildPercentLeft(unitDefinition));
            },
            [&](const CobEnvironment::QueryStatus::YardOpen&) {
                const auto& unit = sim.getUnitState(unitId);
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
                return 0; // TODO
            },
            [&](const CobEnvironment::QueryStatus::MyId&) {
                return static_cast<int>(unitId.value);
            },
            [&](const CobEnvironment::QueryStatus::UnitTeam& q) {
                auto targetUnitId = q.targetUnitId;
                auto targetUnitOption = sim.tryGetUnitState(targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: unsure if correct return value when unit does not exist
                    return 0;
                }
                // TODO: return player's team instead of player ID
                return static_cast<int>(targetUnitOption->get().owner.value);
            },
            [&](const CobEnvironment::QueryStatus::UnitBuildPercentLeft& q) {
                auto targetUnitOption = sim.tryGetUnitState(q.targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: unsure if correct return value when unit does not exist
                    return 0;
                }
                const auto& unitDefinition = sim.unitDefinitions.at(targetUnitOption->get().unitType);
                return static_cast<int>(targetUnitOption->get().getBuildPercentLeft(unitDefinition));
            },
            [&](const CobEnvironment::QueryStatus::UnitAllied& q) {
                const auto& unit = sim.getUnitState(unitId);
                auto targetUnitOption = sim.tryGetUnitState(q.targetUnitId);
                if (!targetUnitOption)
                {
                    // FIXME: unsure if correct return value when unit does not exist
                    return 0;
                }
                // TODO: real allied check including teams/alliances
                return static_cast<int>(targetUnitOption->get().isOwnedBy(unit.owner));
            });
    }

    void handleSetQuery(GameSimulation& sim, const CobEnvironment& env, UnitId unitId, const CobEnvironment::SetQueryStatus& result)
    {
        match(
            result.query,
            [&](const CobEnvironment::SetQueryStatus::Activation& q) {
                if (q.value)
                {
                    sim.activateUnit(unitId);
                }
                else
                {
                    sim.deactivateUnit(unitId);
                }
            },
            [&](const CobEnvironment::SetQueryStatus::StandingMoveOrders&) {
                // TODO
            },
            [&](const CobEnvironment::SetQueryStatus::StandingFireOrders&) {
                // TODO
            },
            [&](const CobEnvironment::SetQueryStatus::InBuildStance& q) {
                sim.setBuildStance(unitId, q.value);
            },
            [&](const CobEnvironment::SetQueryStatus::Busy&) {
                // TODO
            },
            [&](const CobEnvironment::SetQueryStatus::YardOpen& q) {
                sim.setYardOpen(unitId, q.value);
            },
            [&](const CobEnvironment::SetQueryStatus::BuggerOff& q) {
                sim.setBuggerOff(unitId, q.value);
            },
            [&](const CobEnvironment::SetQueryStatus::Armored&) {
                // TODO
            });
    }

    void runUnitCobScripts(GameSimulation& simulation, UnitId unitId)
    {
        auto& unit = simulation.getUnitState(unitId);
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
                    return !simulation.isPieceMoving(unitId, pieceName, toSimAxis(condition.axis));
                },
                [&env, &simulation, unitId](const CobEnvironment::BlockedStatus::Turn& condition) {
                    const auto& pieceName = env._script->pieces.at(condition.object);
                    return !simulation.isPieceTurning(unitId, pieceName, toSimAxis(condition.axis));
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
        auto cobTime = toCobTime(simulation.gameTime);
        for (auto it = env.sleepingQueue.begin(); it != env.sleepingQueue.end();)
        {
            const auto& pair = *it;
            const auto& wakeTime = pair.first;
            if (cobTime >= wakeTime)
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
                    handlePieceCommand(simulation, env, unitId, s);
                },
                [&](const CobEnvironment::QueryStatus& s) {
                    auto result = handleQuery(simulation, env, unitId, s);
                    env.pushResult(result);
                },
                [&](const CobEnvironment::SetQueryStatus& s) {
                    handleSetQuery(simulation, env, unitId, s);
                });
        }

        assert(env.isNotCorrupt());
    }
}
