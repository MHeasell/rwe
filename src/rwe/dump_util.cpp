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
    nlohmann::json dumpJson(const Unit& u)
    {
        return nlohmann::json{
            {"unitType", dumpJson(u.unitType)},
            {"position", dumpJson(u.position)},
            {"owner", dumpJson(u.owner)},
            {"rotation", dumpJson(u.rotation)},
            {"turnRate", dumpJson(u.turnRate)},
            {"currentSpeed", dumpJson(u.currentSpeed)},
            {"targetAngle", dumpJson(u.targetAngle)},
            {"targetSpeed", dumpJson(u.targetSpeed)},
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
    nlohmann::json dumpJson(const Vector3f& v)
    {
        return nlohmann::json{
            {"x", v.x},
            {"y", v.y},
            {"z", v.z},
        };
    }
    nlohmann::json dumpJson(const LaserProjectile& laser)
    {
        nlohmann::json j{
            {"owner", dumpJson(laser.owner)},
            {"position", dumpJson(laser.position)},
            {"origin", dumpJson(laser.origin)},
            {"velocity", dumpJson(laser.velocity)},
            {"damageRadius", dumpJson(laser.damageRadius)}};

        for (const auto& [t, damage] : laser.damage)
        {
            j["damage"][t] = dumpJson(damage);
        }

        return j;
    }
    nlohmann::json dumpJson(const IdleState&)
    {
        return nlohmann::json();
    }
    nlohmann::json dumpJson(const BuildingState&)
    {
        return nlohmann::json();
    }
    nlohmann::json dumpJson(const MovingState& m)
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
            {"lasers", dumpJson(simulation.lasers)},
        };
    }
}
