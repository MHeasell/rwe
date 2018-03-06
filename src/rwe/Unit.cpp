#include "Unit.h"
#include <rwe/GameScene.h>
#include <rwe/geometry/Plane3f.h>
#include <rwe/math/rwe_math.h>

namespace rwe
{
    MoveOrder::MoveOrder(const Vector3f& destination) : destination(destination)
    {
    }

    UnitOrder createMoveOrder(const Vector3f& destination)
    {
        return MoveOrder(destination);
    }

    UnitOrder createAttackOrder(UnitId target)
    {
        return AttackOrder(target);
    }

    UnitOrder createAttackGroundOrder(const Vector3f& target)
    {
        return AttackGroundOrder(target);
    }

    Unit::Unit(const UnitMesh& mesh, std::unique_ptr<CobEnvironment>&& cobEnvironment, SelectionMesh&& selectionMesh)
        : mesh(mesh), cobEnvironment(std::move(cobEnvironment)), selectionMesh(std::move(selectionMesh))
    {
    }

    void Unit::moveObject(const std::string& pieceName, Axis axis, float targetPosition, float speed)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        UnitMesh::MoveOperation op(targetPosition, speed);

        switch (axis)
        {
            case Axis::X:
                piece->xMoveOperation = op;
                break;
            case Axis::Y:
                piece->yMoveOperation = op;
                break;
            case Axis::Z:
                piece->zMoveOperation = op;
                break;
        }
    }

    void Unit::moveObjectNow(const std::string& pieceName, Axis axis, float targetPosition)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                piece->offset.x = targetPosition;
                piece->xMoveOperation = boost::none;
                break;
            case Axis::Y:
                piece->offset.y = targetPosition;
                piece->yMoveOperation = boost::none;
                break;
            case Axis::Z:
                piece->offset.z = targetPosition;
                piece->zMoveOperation = boost::none;
                break;
        }
    }

    void Unit::turnObject(const std::string& pieceName, Axis axis, RadiansAngle targetAngle, float speed)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        UnitMesh::TurnOperation op(targetAngle, toRadians(speed));

        switch (axis)
        {
            case Axis::X:
                piece->xTurnOperation = op;
                break;
            case Axis::Y:
                piece->yTurnOperation = op;
                break;
            case Axis::Z:
                piece->zTurnOperation = op;
                break;
        }
    }

    void Unit::turnObjectNow(const std::string& pieceName, Axis axis, RadiansAngle targetAngle)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                piece->rotation.x = targetAngle.value;
                piece->xTurnOperation = boost::none;
                break;
            case Axis::Y:
                piece->rotation.y = targetAngle.value;
                piece->yTurnOperation = boost::none;
                break;
            case Axis::Z:
                piece->rotation.z = targetAngle.value;
                piece->zTurnOperation = boost::none;
                break;
        }
    }

    bool Unit::isMoveInProgress(const std::string& pieceName, Axis axis) const
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                return !!(piece->xMoveOperation);
            case Axis::Y:
                return !!(piece->yMoveOperation);
            case Axis::Z:
                return !!(piece->zMoveOperation);
        }

        throw std::logic_error("Invalid axis");
    }

    bool Unit::isTurnInProgress(const std::string& pieceName, Axis axis) const
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                return !!(piece->xTurnOperation);
            case Axis::Y:
                return !!(piece->yTurnOperation);
            case Axis::Z:
                return !!(piece->zTurnOperation);
        }

        throw std::logic_error("Invalid axis");
    }

    boost::optional<float> Unit::selectionIntersect(const Ray3f& ray) const
    {
        auto line = ray.toLine();
        Line3f modelSpaceLine(line.start - position, line.end - position);
        auto v = selectionMesh.collisionMesh.intersectLine(modelSpaceLine);
        if (!v)
        {
            return boost::none;
        }

        return ray.origin.distance(*v);
    }

    bool Unit::isOwnedBy(PlayerId playerId) const
    {
        return owner == playerId;
    }

    void Unit::clearOrders()
    {
        orders.clear();
        behaviourState = IdleState();

        // not clear if this really belongs here
        clearWeaponTargets();
    }

    void Unit::addOrder(const UnitOrder& order)
    {
        orders.push_back(order);
    }

    class TargetIsUnitVisitor : public boost::static_visitor<bool>
    {
    private:
        UnitId unit;

    public:
        TargetIsUnitVisitor(UnitId unit) : unit(unit) {}
        bool operator()(UnitId target) const { return unit == target; }
        bool operator()(const Vector3f& target) const { return false; }
    };

    class IsAttackingUnitVisitor : public boost::static_visitor<bool>
    {
    private:
        UnitId unit;

    public:
        IsAttackingUnitVisitor(const UnitId& unit) : unit(unit) {}
        bool operator()(const UnitWeaponStateIdle&) const { return false; }
        bool operator()(const UnitWeaponStateAttacking& state) const { return boost::apply_visitor(TargetIsUnitVisitor(unit), state.target); }
    };


    class TargetIsPositionVisitor : public boost::static_visitor<bool>
    {
    private:
        Vector3f position;

    public:
        TargetIsPositionVisitor(const Vector3f& position) : position(position) {}
        bool operator()(UnitId target) const { return false; }
        bool operator()(const Vector3f& target) const { return target == position; }
    };

    class IsAttackingPositionVisitor : public boost::static_visitor<bool>
    {
    private:
        Vector3f position;

    public:
        IsAttackingPositionVisitor(const Vector3f& position) : position(position) {}
        bool operator()(const UnitWeaponStateIdle&) const { return false; }
        bool operator()(const UnitWeaponStateAttacking& state) const { return boost::apply_visitor(TargetIsPositionVisitor(position), state.target); }
    };

    void Unit::setWeaponTarget(unsigned int weaponIndex, UnitId target)
    {
        auto& weapon = weapons[weaponIndex];
        if (!boost::apply_visitor(IsAttackingUnitVisitor(target), weapon.state))
        {
            clearWeaponTarget(weaponIndex);
            weapon.state = UnitWeaponStateAttacking(target);
        }
    }

    void Unit::setWeaponTarget(unsigned int weaponIndex, const Vector3f& target)
    {
        auto& weapon = weapons[weaponIndex];
        if (!boost::apply_visitor(IsAttackingPositionVisitor(target), weapon.state))
        {
            clearWeaponTarget(weaponIndex);
            weapon.state = UnitWeaponStateAttacking(target);
        }
    }

    void Unit::clearWeaponTarget(unsigned int weaponIndex)
    {
        auto& weapon = weapons[weaponIndex];
        weapon.state = UnitWeaponStateIdle();
        cobEnvironment->createThread("TargetCleared", {static_cast<int>(weaponIndex)});
    }

    void Unit::clearWeaponTargets()
    {
        for (unsigned int i = 0; i < weapons.size(); ++i)
        {
            clearWeaponTarget(i);
        }
    }
}
