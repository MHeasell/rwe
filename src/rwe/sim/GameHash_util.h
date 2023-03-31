#pragma once

#include <cstdint>
#include <rwe/sim/GameHash.h>
#include <rwe/sim/GameSimulation.h>
#include <rwe/util/match.h>

namespace rwe
{
    GameHash computeHashOf(GameHash hash);

    GameHash computeHashOf(float f);

    GameHash computeHashOf(bool b);

    GameHash computeHashOf(uint32_t i);

    GameHash computeHashOf(int32_t i);

    GameHash computeHashOf(const std::string& s);

    GameHash computeHashOf(const char* s);

    GameHash computeHashOf(const GamePlayerInfo& p);

    GameHash computeHashOf(const UnitState& u);

    GameHash computeHashOf(const UnitPhysicsInfoGround& p);
    GameHash computeHashOf(const UnitPhysicsInfoAir& p);
    GameHash computeHashOf(const AirMovementStateTakingOff& p);
    GameHash computeHashOf(const AirMovementStateLanding& p);
    GameHash computeHashOf(const AirMovementStateFlying& p);

    GameHash computeHashOf(const SteeringInfo& s);

    GameHash computeHashOf(const Projectile& projectile);

    GameHash computeHashOf(const UnitBehaviorStateIdle&);

    GameHash computeHashOf(const UnitBehaviorStateBuilding& s);

    GameHash computeHashOf(const UnitBehaviorStateCreatingUnit&);


    GameHash computeHashOf(const UnitCreationStatusPending&);
    GameHash computeHashOf(const UnitCreationStatusDone&);
    GameHash computeHashOf(const UnitCreationStatusFailed&);

    GameHash computeHashOf(const UnitState::LifeStateAlive&);
    GameHash computeHashOf(const UnitState::LifeStateDead&);

    GameHash computeHashOf(const NavigationGoalLandingLocation&);

    GameHash computeHashOf(const NavigationStateIdle&);
    GameHash computeHashOf(const NavigationStateMoving& m);
    GameHash computeHashOf(const NavigationStateMovingToLandingSpot&);
    GameHash computeHashOf(const NavigationStateInfo& i);

    GameHash computeHashOf(const DiscreteRect& r);

    GameHash computeHashOf(const GameSimulation& simulation);

    template <typename... Ts>
    GameHash combineHashes(const Ts&... items);

    template <typename Val>
    GameHash computeHashOf(const Vector3x<Val>& v)
    {
        return combineHashes(v.x, v.y, v.z);
    }

    template <typename T, typename Tag>
    GameHash computeHashOf(const OpaqueId<T, Tag>& id)
    {
        return computeHashOf(id.value);
    }

    template <typename T>
    GameHash computeHashOf(const std::optional<T>& o)
    {
        return o ? computeHashOf(*o) : GameHash(0);
    }

    template <typename T>
    GameHash computeHashOf(const std::vector<T>& v)
    {
        GameHash sum(0);
        for (const auto& x : v)
        {
            sum += computeHashOf(x);
        }
        return sum;
    }

    template <typename T, typename Tag>
    GameHash computeHashOf(const VectorMap<T, Tag>& v)
    {
        GameHash sum(0);
        for (const auto& x : v)
        {
            sum += computeHashOf(x);
        }
        return sum;
    }

    template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    GameHash computeHashOf(const T& en)
    {
        return GameHash(static_cast<std::underlying_type_t<T>>(en));
    }

    template <typename... Ts>
    GameHash computeHashOf(const std::variant<Ts...>& v)
    {
        return GameHash(v.index())
            + match(v, [](const auto& x) { return computeHashOf(x); });
    }

    template <typename A, typename B>
    GameHash computeHashOf(const std::pair<A, B>& p)
    {
        return computeHashOf(p.first) + computeHashOf(p.second);
    }

    template <typename... Ts>
    GameHash combineHashes(const Ts&... items)
    {
        uint32_t sum = 0;
        ((sum = sum + computeHashOf(items).value), ...);
        return GameHash(sum);
    }
}
