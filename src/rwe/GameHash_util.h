#pragma once

#include <cstdint>
#include <rwe/GameHash.h>
#include <rwe/match.h>
#include <rwe/sim/GameSimulation.h>

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

    GameHash computeHashOf(const Unit& u);

    GameHash computeHashOf(const Projectile& projectile);

    GameHash computeHashOf(const IdleState&);

    GameHash computeHashOf(const BuildingState& s);

    GameHash computeHashOf(const CreatingUnitState&);

    GameHash computeHashOf(const UnitCreationStatusPending&);
    GameHash computeHashOf(const UnitCreationStatusDone&);
    GameHash computeHashOf(const UnitCreationStatusFailed&);

    GameHash computeHashOf(const MovingState& m);

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
        return GameHash(static_cast<uint32_t>(v.index()))
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
