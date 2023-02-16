#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <rwe/OpaqueId.h>
#include <rwe/sim/GameSimulation.h>
#include <string>
#include <utility>

namespace rwe
{
    nlohmann::json dumpJson(float f);

    nlohmann::json dumpJson(bool b);

    nlohmann::json dumpJson(uint32_t i);

    nlohmann::json dumpJson(int32_t i);

    nlohmann::json dumpJson(const std::string& s);

    nlohmann::json dumpJson(const char* s);

    nlohmann::json dumpJson(const GamePlayerInfo& p);

    nlohmann::json dumpJson(const UnitState& u);

    nlohmann::json dumpJson(const UnitPhysicsInfoGround& p);
    nlohmann::json dumpJson(const UnitPhysicsInfoAir& p);
    nlohmann::json dumpJson(const AirMovementStateTakingOff& p);
    nlohmann::json dumpJson(const AirMovementStateLanding& p);
    nlohmann::json dumpJson(const AirMovementStateFlying& p);

    nlohmann::json dumpJson(const SteeringInfo& s);

    nlohmann::json dumpJson(const Vector3f& v);

    template <typename Val>
    nlohmann::json dumpJson(const Vector3x<Val>& v)
    {
        return nlohmann::json{
            {"x", dumpJson(v.x)},
            {"y", dumpJson(v.y)},
            {"z", dumpJson(v.z)},
        };
    }

    nlohmann::json dumpJson(const Projectile& projectile);

    nlohmann::json dumpJson(const UnitBehaviorStateIdle&);

    nlohmann::json dumpJson(const UnitBehaviorStateBuilding&);

    nlohmann::json dumpJson(const UnitBehaviorStateCreatingUnit&);

    nlohmann::json dumpJson(const UnitCreationStatusPending&);
    nlohmann::json dumpJson(const UnitCreationStatusDone&);
    nlohmann::json dumpJson(const UnitCreationStatusFailed&);

    nlohmann::json dumpJson(const UnitState::LifeStateAlive&);
    nlohmann::json dumpJson(const UnitState::LifeStateDead&);

    nlohmann::json dumpJson(const NavigationGoalLandingLocation& m);

    nlohmann::json dumpJson(const NavigationStateIdle& m);
    nlohmann::json dumpJson(const NavigationStateMoving& m);
    nlohmann::json dumpJson(const NavigationStateMovingToLandingSpot& m);
    nlohmann::json dumpJson(const NavigationStateInfo& m);

    nlohmann::json dumpJson(const DiscreteRect& r);

    nlohmann::json dumpJson(const GameSimulation& simulation);

    template <typename T, typename Tag>
    nlohmann::json dumpJson(const OpaqueId<T, Tag>& id)
    {
        return dumpJson(id.value);
    }

    template <typename T>
    nlohmann::json dumpJson(const std::optional<T>& o)
    {
        return o ? dumpJson(*o) : nlohmann::json();
    }

    template <typename T>
    nlohmann::json dumpJson(const std::vector<T>& v)
    {
        nlohmann::json j;
        for (const auto& e : v)
        {
            j.push_back(dumpJson(e));
        }
        return j;
    }

    template <typename A, typename B>
    nlohmann::json dumpJson(const std::pair<A, B>& p)
    {
        return nlohmann::json{
            {"first", dumpJson(p.first)},
            {"second", dumpJson(p.second)}};
    }

    template <typename T, typename Tag>
    nlohmann::json dumpJson(const VectorMap<T, Tag>& v)
    {
        nlohmann::json j;
        for (const auto& e : v)
        {
            j.push_back(dumpJson(e));
        }
        return j;
    }

    template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    nlohmann::json dumpJson(const T& en)
    {
        return nlohmann::json(static_cast<std::underlying_type_t<T>>(en));
    }

    template <typename... Ts>
    nlohmann::json dumpJson(const std::variant<Ts...>& v)
    {
        nlohmann::json j;
        j["variant"] = v.index();
        j["data"] = match(v, [](const auto& x) { return dumpJson(x); });
        return j;
    }
}
