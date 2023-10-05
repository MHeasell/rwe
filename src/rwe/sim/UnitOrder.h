#pragma once

#include <rwe/grid/DiscreteRect.h>
#include <rwe/sim/FeatureId.h>
#include <rwe/sim/SimVector.h>
#include <rwe/sim/UnitId.h>
#include <variant>

namespace rwe
{
    struct MoveOrder
    {
        SimVector destination;
        explicit MoveOrder(const SimVector& destination) : destination(destination) {}
    };

    using AttackTarget = std::variant<UnitId, SimVector>;

    struct AttackOrder
    {
        AttackTarget target;
        explicit AttackOrder(UnitId target) : target(target) {}
        explicit AttackOrder(const SimVector& target) : target(target) {}
    };

    struct BuildOrder
    {
        std::string unitType;
        SimVector position;
        BuildOrder(const std::string& unitType, const SimVector& position) : unitType(unitType), position(position) {}
    };

    struct BuggerOffOrder
    {
        DiscreteRect rect;
        explicit BuggerOffOrder(const DiscreteRect& r) : rect(r) {}
    };

    /** Finish building an already in-progress unit */
    struct CompleteBuildOrder
    {
        UnitId target;
        explicit CompleteBuildOrder(const UnitId& target) : target(target) {}
    };

    struct GuardOrder
    {
        UnitId target;
        explicit GuardOrder(const UnitId& target) : target(target) {}
    };

    struct ReclaimOrder
    {
        std::variant<UnitId, FeatureId> target;
        explicit ReclaimOrder(const UnitId& target) : target(target) {}
        explicit ReclaimOrder(const FeatureId& target) : target(target) {}
    };

    using UnitOrder = std::variant<MoveOrder, AttackOrder, BuildOrder, BuggerOffOrder, CompleteBuildOrder, GuardOrder, ReclaimOrder>;
}
