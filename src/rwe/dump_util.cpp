#include "dump_util.h"

namespace rwe
{
    nlohmann::json dumpJson(float f)
    {
        return f;
    }
    nlohmann::json dumpJson(bool b)
    {
        return b;
    }
    nlohmann::json dumpJson(uint32_t i)
    {
        return i;
    }
    nlohmann::json dumpJson(int32_t i)
    {
        return i;
    }
    nlohmann::json dumpJson(const std::string& s)
    {
        return s;
    }
    nlohmann::json dumpJson(const char* s)
    {
        return s;
    }
    nlohmann::json dumpJson(const GamePlayerInfo& p)
    {
        return nlohmann::json{
            {"type", dumpJson(p.type)},
            {"color", dumpJson(p.color)},
            {"status", dumpJson(p.status)},
            {"side", dumpJson(p.side)},
            {"metal", dumpJson(p.metal)},
            {"maxMetal", dumpJson(p.maxMetal)},
            {"energy", dumpJson(p.energy)},
            {"maxEnergy", dumpJson(p.maxEnergy)},
            {"metalStalled", dumpJson(p.metalStalled)},
            {"energyStalled", dumpJson(p.energyStalled)},
            {"desiredMetalConsumptionBuffer", dumpJson(p.desiredMetalConsumptionBuffer)},
            {"desiredEnergyConsumptionBuffer", dumpJson(p.desiredEnergyConsumptionBuffer)},
            {"previousDesiredMetalConsumptionBuffer", dumpJson(p.previousDesiredMetalConsumptionBuffer)},
            {"previousDesiredEnergyConsumptionBuffer", dumpJson(p.previousDesiredEnergyConsumptionBuffer)},
            {"actualMetalConsumptionBuffer", dumpJson(p.actualMetalConsumptionBuffer)},
            {"actualEnergyConsumptionBuffer", dumpJson(p.actualEnergyConsumptionBuffer)},
            {"metalProductionBuffer", dumpJson(p.metalProductionBuffer)},
            {"energyProductionBuffer", dumpJson(p.energyProductionBuffer)}};
    }
    nlohmann::json dumpJson(const UnitState& u)
    {
        return nlohmann::json{
            {"unitType", dumpJson(u.unitType)},
            {"position", dumpJson(u.position)},
            {"owner", dumpJson(u.owner)},
            {"rotation", dumpJson(u.rotation)},
            {"physics", dumpJson(u.physics)},
            {"hitPoints", dumpJson(u.hitPoints)},
            {"lifeState", dumpJson(u.lifeState)},
            {"behaviourState", dumpJson(u.behaviourState)},
            {"inBuildStance", dumpJson(u.inBuildStance)},
            {"yardOpen", dumpJson(u.yardOpen)},
            {"inCollision", dumpJson(u.inCollision)},
            {"fireOrders", dumpJson(u.fireOrders)},
            {"buildTimeCompleted", dumpJson(u.buildTimeCompleted)},
            {"activated", dumpJson(u.activated)},
            {"isSufficientlyPowered", dumpJson(u.isSufficientlyPowered)},
            {"energyProductionBuffer", dumpJson(u.energyProductionBuffer)},
            {"metalProductionBuffer", dumpJson(u.metalProductionBuffer)},
            {"previousEnergyConsumptionBuffer", dumpJson(u.previousEnergyConsumptionBuffer)},
            {"previousMetalConsumptionBuffer", dumpJson(u.previousMetalConsumptionBuffer)},
            {"energyConsumptionBuffer", dumpJson(u.energyConsumptionBuffer)},
            {"metalConsumptionBuffer", dumpJson(u.metalConsumptionBuffer)}};
    }

    nlohmann::json dumpJson(const GroundPhysics& p)
    {
        return nlohmann::json{
            {"steeringInfo", dumpJson(p.steeringInfo)},
            {"currentSpeed", dumpJson(p.currentSpeed)},
        };
    }

    nlohmann::json dumpJson(const AirPhysics& p)
    {
        return nlohmann::json{
            {"movementState", dumpJson(p.movementState)},
        };
    }

    nlohmann::json dumpJson(const AirTakingOffPhysics& p)
    {
        return nlohmann::json{};
    }

    nlohmann::json dumpJson(const AirLandingPhysics& p)
    {
        return nlohmann::json{};
    }

    nlohmann::json dumpJson(const AirFlyingPhysics& p)
    {
        return nlohmann::json{};
    }

    nlohmann::json dumpJson(const SteeringInfo& s)
    {
        return nlohmann::json{
            {"targetAngle", dumpJson(s.targetAngle)},
            {"targetSpeed", dumpJson(s.targetSpeed)},
        };
    }

    nlohmann::json dumpJson(const Vector3f& v)
    {
        return nlohmann::json{
            {"x", v.x},
            {"y", v.y},
            {"z", v.z},
        };
    }
    nlohmann::json dumpJson(const Projectile& projectile)
    {
        nlohmann::json j{
            {"owner", dumpJson(projectile.owner)},
            {"position", dumpJson(projectile.position)},
            {"origin", dumpJson(projectile.origin)},
            {"velocity", dumpJson(projectile.velocity)},
            {"damageRadius", dumpJson(projectile.damageRadius)}};

        for (const auto& [t, damage] : projectile.damage)
        {
            j["damage"][t] = dumpJson(damage);
        }

        return j;
    }
    nlohmann::json dumpJson(const UnitBehaviorStateIdle&)
    {
        return nlohmann::json();
    }
    nlohmann::json dumpJson(const UnitBehaviorStateBuilding&)
    {
        return nlohmann::json();
    }
    nlohmann::json dumpJson(const UnitBehaviorStateCreatingUnit& s)
    {
        return nlohmann::json{
            {"unitType", dumpJson(s.unitType)},
            {"owner", dumpJson(s.owner)},
            {"position", dumpJson(s.position)},
        };
    }
    nlohmann::json dumpJson(const UnitBehaviorStateFlyingToLandingSpot& s)
    {
        return nlohmann::json();
    }
    nlohmann::json dumpJson(const UnitCreationStatusPending&)
    {
        return nlohmann::json();
    }
    nlohmann::json dumpJson(const UnitCreationStatusDone& s)
    {
        return nlohmann::json{{"unitId", dumpJson(s.unitId)}};
    }
    nlohmann::json dumpJson(const UnitCreationStatusFailed&)
    {
        return nlohmann::json();
    }

    nlohmann::json dumpJson(const UnitState::LifeStateAlive&)
    {
        return nlohmann::json();
    }
    nlohmann::json dumpJson(const UnitState::LifeStateDead&)
    {
        return nlohmann::json();
    }

    nlohmann::json dumpJson(const UnitBehaviorStateMoving& m)
    {
        return nlohmann::json{
            {"destination", dumpJson(m.destination)},
            {"pathRequested", m.pathRequested}};
    }
    nlohmann::json dumpJson(const DiscreteRect& r)
    {
        return nlohmann::json{
            {"x", r.x},
            {"y", r.y},
            {"width", r.width},
            {"height", r.height}};
    }
    nlohmann::json dumpJson(const GameSimulation& simulation)
    {
        return nlohmann::json{
            {"gameTime", dumpJson(simulation.gameTime)},
            {"players", dumpJson(simulation.players)},
            {"units", dumpJson(simulation.units)},
            {"projectiles", dumpJson(simulation.projectiles)},
        };
    }
}
