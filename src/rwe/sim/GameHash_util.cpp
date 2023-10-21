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

    GameHash computeHashOf(const UnitState& u)
    {
        return combineHashes(
            u.unitType,
            u.position,
            u.owner,
            u.rotation,
            u.physics,
            u.hitPoints,
            u.lifeState,
            u.navigationState,
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

    GameHash computeHashOf(const UnitPhysicsInfoGround& p)
    {
        return combineHashes(p.steeringInfo, p.currentSpeed);
    }

    GameHash computeHashOf(const UnitPhysicsInfoAir& p)
    {
        return combineHashes(p.movementState);
    }

    GameHash computeHashOf(const AirMovementStateTakingOff& p)
    {
        return GameHash(0);
    }

    GameHash computeHashOf(const AirMovementStateLanding& p)
    {
        return GameHash(0);
    }

    GameHash computeHashOf(const AirMovementStateFlying& p)
    {
        return GameHash(0);
    }

    GameHash computeHashOf(const SteeringInfo& s)
    {
        return combineHashes(s.targetAngle, s.targetSpeed);
    }

    GameHash computeHashOf(const Vector3f& v)
    {
        return combineHashes(v.x, v.y, v.z);
    }

    GameHash computeHashOf(const Projectile& projectile)
    {
        GameHash h = combineHashes(
            projectile.owner,
            projectile.position,
            projectile.origin,
            projectile.velocity,
            projectile.damageRadius);

        for (const auto& [_, damage] : projectile.damage)
        {
            h += computeHashOf(damage);
        }
        return h;
    }

    GameHash computeHashOf(const UnitBehaviorStateIdle&)
    {
        return GameHash(0);
    }

    GameHash computeHashOf(const UnitBehaviorStateBuilding& s)
    {
        return combineHashes(s.targetUnit, s.nanoParticleOrigin);
    }

    GameHash computeHashOf(const UnitBehaviorStateCreatingUnit& s)
    {
        return combineHashes(
            s.position,
            s.owner,
            s.unitType,
            s.status);
    }

    GameHash computeHashOf(const UnitCreationStatusPending&)
    {
        return GameHash(0);
    }
    GameHash computeHashOf(const UnitCreationStatusDone& s)
    {
        return combineHashes(s.unitId);
    }

    GameHash computeHashOf(const UnitCreationStatusFailed&)
    {
        return GameHash(0);
    }

    GameHash computeHashOf(const UnitState::LifeStateAlive&)
    {
        return GameHash(0);
    }
    GameHash computeHashOf(const UnitState::LifeStateDead&)
    {
        return GameHash(0);
    }

    GameHash computeHashOf(const NavigationGoalLandingLocation&)
    {
        return GameHash(0);
    }

    GameHash computeHashOf(const NavigationStateIdle&)
    {
        return GameHash(0);
    }

    GameHash computeHashOf(const NavigationStateMoving& m)
    {
        return combineHashes(
            m.pathDestination,
            m.pathRequested);
    }

    GameHash computeHashOf(const NavigationStateMovingToLandingSpot& m)
    {
        return combineHashes(m.landingLocation);
    }

    GameHash computeHashOf(const NavigationStateInfo& i)
    {
        return combineHashes(i.desiredDestination, i.state);
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
            simulation.projectiles);
    }
}
