#ifndef RWE_UNITORDER_H
#define RWE_UNITORDER_H

#include <boost/variant.hpp>
#include <rwe/UnitId.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    struct MoveOrder
    {
        Vector3f destination;
        explicit MoveOrder(const Vector3f& destination);
    };

    using AttackTarget = boost::variant<UnitId, Vector3f>;

    struct AttackOrder
    {
        AttackTarget target;
        explicit AttackOrder(UnitId target) : target(target) {}
        explicit AttackOrder(const Vector3f& target) : target(target) {}
    };

    struct BuildOrder
    {
        std::string unitType;
        Vector3f position;
        BuildOrder(const std::string& unitType, const Vector3f& position) : unitType(unitType), position(position) {}
    };

    using UnitOrder = boost::variant<MoveOrder, AttackOrder, BuildOrder>;
}

#endif
