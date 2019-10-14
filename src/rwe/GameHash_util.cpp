#include "GameHash_util.h"

namespace rwe
{
    GameHash computeHashOf(GameHash hash)
    {
        return hash;
    }

    GameHash computeHashOf(float f)
    {
        return GameHash(static_cast<uint32_t>(f * (1u << 16u)));
    }

    GameHash computeHashOf(bool b)
    {
        return b ? GameHash(1) : GameHash(0);
    }

    GameHash computeHashOf(uint32_t i)
    {
        return GameHash(i);
    }

    GameHash computeHashOf(int32_t i)
    {
        return GameHash(static_cast<uint32_t>(i));
    }

    GameHash computeHashOf(const std::string& s)
    {
        uint32_t sum = 0;
        for (const char& c : s)
        {
            sum += c;
        }
        return GameHash(sum);
    }

    GameHash computeHashOf(const char* s)
    {
        uint32_t sum = 0;
        for (; *s != '\0'; ++s)
        {
            sum += *s;
        }
        return GameHash(sum);
    }

    GameHash computeHashOf(const GamePlayerInfo& p)
    {
        return combineHashes(
            p.type,
            p.color,
            p.status,
            p.side,
            p.metal,
            p.maxMetal,
            p.energy,
            p.maxEnergy,
            p.metalStalled,
            p.energyStalled,
            p.desiredMetalConsumptionBuffer,
            p.desiredEnergyConsumptionBuffer,
            p.previousDesiredMetalConsumptionBuffer,
            p.previousDesiredEnergyConsumptionBuffer,
            p.actualMetalConsumptionBuffer,
            p.actualEnergyConsumptionBuffer,
            p.metalProductionBuffer,
            p.energyProductionBuffer);
    }

    GameHash computeHashOf(const Unit& u)
    {
        return combineHashes(
            u.unitType,
            u.position,
            u.owner,
            u.rotation,
            u.turnRate,
            u.currentSpeed,
            u.targetAngle,
            u.targetSpeed,
            u.hitPoints,
            u.lifeState,
            u.behaviourState,
            u.inBuildStance,
            u.yardOpen,
            u.inCollision,
            u.fireOrders,
            u.buildTimeCompleted,
            u.activated,
            u.isSufficientlyPowered,
            u.energyProductionBuffer,
            u.metalProductionBuffer,
            u.previousEnergyConsumptionBuffer,
            u.previousMetalConsumptionBuffer,
            u.energyConsumptionBuffer,
            u.metalConsumptionBuffer);
    }

    GameHash computeHashOf(const Vector3f& v)
    {
        return combineHashes(v.x, v.y, v.z);
    }

    GameHash computeHashOf(const LaserProjectile& laser)
    {
        GameHash h = combineHashes(
            laser.owner,
            laser.position,
            laser.origin,
            laser.velocity,
            laser.damageRadius);

        for (const auto& [_, damage] : laser.damage)
        {
            h += computeHashOf(damage);
        }
        return h;
    }

    GameHash computeHashOf(const IdleState&)
    {
        return GameHash(0);
    }

    GameHash computeHashOf(const BuildingState&)
    {
        return GameHash(0);
    }

    GameHash computeHashOf(const MovingState& m)
    {
        return combineHashes(
            m.destination,
            m.pathRequested);
    }

    GameHash computeHashOf(const DiscreteRect& r)
    {
        return combineHashes(r.x, r.y, r.width, r.height);
    }

    GameHash computeHashOf(const GameSimulation& simulation)
    {
        return combineHashes(
            simulation.gameTime,
            simulation.players,
            simulation.units,
            simulation.lasers);
    }
}
