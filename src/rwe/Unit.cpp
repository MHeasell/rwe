#include "Unit.h"
#include <rwe/GameScene.h>
#include <rwe/geometry/Plane3f.h>
#include <rwe/math/rwe_math.h>
#include <rwe/matrix_util.h>
#include <rwe/unit_util.h>

namespace rwe
{
    UnitOrder createMoveOrder(const SimVector& destination)
    {
        return MoveOrder(destination);
    }

    UnitOrder createAttackOrder(UnitId target)
    {
        return AttackOrder(target);
    }

    UnitOrder createAttackGroundOrder(const SimVector& target)
    {
        return AttackOrder(target);
    }

    bool isPassable(YardMapCell cell, bool yardMapOpen)
    {
        switch (cell)
        {
            case YardMapCell::GroundPassableWhenOpen:
                return yardMapOpen;
            case YardMapCell::WaterPassableWhenOpen:
                return yardMapOpen;
            case YardMapCell::GroundNoFeature:
                return false;
            case YardMapCell::GroundGeoPassableWhenOpen:
                return yardMapOpen;
            case YardMapCell::Geo:
                return false;
            case YardMapCell::Ground:
                return false;
            case YardMapCell::GroundPassableWhenClosed:
                return !yardMapOpen;
            case YardMapCell::Water:
                return false;
            case YardMapCell::GroundPassable:
                return true;
            case YardMapCell::WaterPassable:
                return true;
            case YardMapCell::Passable:
                return true;
            default:
                throw std::logic_error("Unknown cell type");
        }
    }

    SimAngle Unit::toRotation(const SimVector& direction)
    {
        return atan2(direction.x, direction.z);
    }

    SimVector Unit::toDirection(SimAngle rotation)
    {
        return SimVector(sin(rotation), 0_ss, cos(rotation));
    }

    Unit::Unit(const UnitMesh& mesh, std::unique_ptr<CobEnvironment>&& cobEnvironment, SelectionMesh&& selectionMesh)
        : mesh(mesh), cobEnvironment(std::move(cobEnvironment)), selectionMesh(std::move(selectionMesh))
    {
    }

    bool Unit::isBeingBuilt() const
    {
        return buildTimeCompleted < buildTime;
    }

    unsigned int Unit::getBuildPercentLeft() const
    {
        return 100u - ((buildTimeCompleted * 100u) / buildTime);
    }

    float Unit::getPreciseCompletePercent() const
    {
        return static_cast<float>(buildTimeCompleted) / static_cast<float>(buildTime);
    }

    Unit::BuildCostInfo Unit::getBuildCostInfo(unsigned int buildTimeContribution)
    {
        auto remainingBuildTime = buildTime - buildTimeCompleted;
        if (buildTimeContribution > remainingBuildTime)
        {
            buildTimeContribution = remainingBuildTime;
        }

        auto oldProgressEnergy = Energy((buildTimeCompleted * energyCost.value) / buildTime);
        auto oldProgressMetal = Metal((buildTimeCompleted * metalCost.value) / buildTime);

        auto newBuildTimeCompleted = buildTimeCompleted + buildTimeContribution;

        auto newProgressEnergy = Energy((newBuildTimeCompleted * energyCost.value) / buildTime);
        auto newProgressMetal = Metal((newBuildTimeCompleted * metalCost.value) / buildTime);

        auto deltaEnergy = newProgressEnergy - oldProgressEnergy;
        auto deltaMetal = newProgressMetal - oldProgressMetal;

        return BuildCostInfo{buildTimeContribution, deltaEnergy, deltaMetal};
    }

    bool Unit::addBuildProgress(unsigned int buildTimeContribution)
    {
        auto remainingBuildTime = buildTime - buildTimeCompleted;
        if (buildTimeContribution > remainingBuildTime)
        {
            buildTimeContribution = remainingBuildTime;
        }

        auto oldProgressHp = (buildTimeCompleted * maxHitPoints) / buildTime;

        buildTimeCompleted += buildTimeContribution;

        auto newProgressHp = (buildTimeCompleted * maxHitPoints) / buildTime;

        auto deltaHp = newProgressHp - oldProgressHp;

        // add HP up to the maximum
        if (hitPoints + deltaHp >= maxHitPoints)
        {
            hitPoints = maxHitPoints;
        }
        else
        {
            hitPoints += deltaHp;
        }

        return buildTimeCompleted == buildTime;
    }

    bool Unit::isCommander() const
    {
        return commander;
    }

    void Unit::moveObject(const std::string& pieceName, Axis axis, SimScalar targetPosition, SimScalar speed)
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
                piece->get().xMoveOperation = op;
                break;
            case Axis::Y:
                piece->get().yMoveOperation = op;
                break;
            case Axis::Z:
                piece->get().zMoveOperation = op;
                break;
        }
    }

    void Unit::moveObjectNow(const std::string& pieceName, Axis axis, SimScalar targetPosition)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                piece->get().offset.x = targetPosition;
                piece->get().xMoveOperation = std::nullopt;
                break;
            case Axis::Y:
                piece->get().offset.y = targetPosition;
                piece->get().yMoveOperation = std::nullopt;
                break;
            case Axis::Z:
                piece->get().offset.z = targetPosition;
                piece->get().zMoveOperation = std::nullopt;
                break;
        }
    }

    void Unit::turnObject(const std::string& pieceName, Axis axis, SimAngle targetAngle, SimScalar speed)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        UnitMesh::TurnOperation op(targetAngle, speed);

        switch (axis)
        {
            case Axis::X:
                piece->get().xTurnOperation = op;
                break;
            case Axis::Y:
                piece->get().yTurnOperation = op;
                break;
            case Axis::Z:
                piece->get().zTurnOperation = op;
                break;
        }
    }

    void Unit::turnObjectNow(const std::string& pieceName, Axis axis, SimAngle targetAngle)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                piece->get().rotationX = targetAngle;
                piece->get().xTurnOperation = std::nullopt;
                break;
            case Axis::Y:
                piece->get().rotationY = targetAngle;
                piece->get().yTurnOperation = std::nullopt;
                break;
            case Axis::Z:
                piece->get().rotationZ = targetAngle;
                piece->get().zTurnOperation = std::nullopt;
                break;
        }
    }

    void Unit::spinObject(const std::string& pieceName, Axis axis, SimScalar speed, SimScalar acceleration)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        UnitMesh::SpinOperation op(acceleration == 0_ss ? speed : 0_ss, speed, acceleration);

        switch (axis)
        {
            case Axis::X:
                piece->get().xTurnOperation = op;
                break;
            case Axis::Y:
                piece->get().yTurnOperation = op;
                break;
            case Axis::Z:
                piece->get().zTurnOperation = op;
                break;
        }
    }

    void setStopSpinOp(std::optional<UnitMesh::TurnOperationUnion>& existingOp, SimScalar deceleration)
    {
        if (!existingOp)
        {
            return;
        }
        auto spinOp = std::get_if<UnitMesh::SpinOperation>(&*existingOp);
        if (spinOp == nullptr)
        {
            return;
        }

        if (deceleration == 0_ss)
        {
            existingOp = std::nullopt;
            return;
        }

        existingOp = UnitMesh::StopSpinOperation(spinOp->currentSpeed, deceleration);
    }

    void Unit::stopSpinObject(const std::string& pieceName, Axis axis, SimScalar deceleration)
    {

        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                setStopSpinOp(piece->get().xTurnOperation, deceleration);
                break;
            case Axis::Y:
                setStopSpinOp(piece->get().yTurnOperation, deceleration);
                break;
            case Axis::Z:
                setStopSpinOp(piece->get().zTurnOperation, deceleration);
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
                return !!(piece->get().xMoveOperation);
            case Axis::Y:
                return !!(piece->get().yMoveOperation);
            case Axis::Z:
                return !!(piece->get().zMoveOperation);
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
                return !!(piece->get().xTurnOperation);
            case Axis::Y:
                return !!(piece->get().yTurnOperation);
            case Axis::Z:
                return !!(piece->get().zTurnOperation);
        }

        throw std::logic_error("Invalid axis");
    }

    std::optional<float> Unit::selectionIntersect(const Ray3f& ray) const
    {
        auto inverseTransform = toFloatMatrix(getInverseTransform());
        auto line = ray.toLine();
        Line3f modelSpaceLine(inverseTransform * line.start, inverseTransform * line.end);
        auto v = selectionMesh.collisionMesh.intersectLine(modelSpaceLine);
        if (!v)
        {
            return std::nullopt;
        }

        return ray.origin.distance(*v);
    }

    bool Unit::isOwnedBy(PlayerId playerId) const
    {
        return owner == playerId;
    }

    bool Unit::isDead() const
    {
        return lifeState == LifeState::Dead;
    }

    void Unit::markAsDead()
    {
        lifeState = LifeState::Dead;
    }

    void Unit::finishBuilding()
    {
        // FIXME: ignores damage taken during building
        hitPoints = maxHitPoints;
        buildTimeCompleted = buildTime;
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

    class TargetIsUnitVisitor
    {
    private:
        UnitId unit;

    public:
        TargetIsUnitVisitor(UnitId unit) : unit(unit) {}
        bool operator()(UnitId target) const { return unit == target; }
        bool operator()(const SimVector&) const { return false; }
    };

    class IsAttackingUnitVisitor
    {
    private:
        UnitId unit;

    public:
        IsAttackingUnitVisitor(const UnitId& unit) : unit(unit) {}
        bool operator()(const UnitWeaponStateIdle&) const { return false; }
        bool operator()(const UnitWeaponStateAttacking& state) const { return std::visit(TargetIsUnitVisitor(unit), state.target); }
    };


    class TargetIsPositionVisitor
    {
    private:
        SimVector position;

    public:
        TargetIsPositionVisitor(const SimVector& position) : position(position) {}
        bool operator()(UnitId) const { return false; }
        bool operator()(const SimVector& target) const { return target == position; }
    };

    class IsAttackingPositionVisitor
    {
    private:
        SimVector position;

    public:
        IsAttackingPositionVisitor(const SimVector& position) : position(position) {}
        bool operator()(const UnitWeaponStateIdle&) const { return false; }
        bool operator()(const UnitWeaponStateAttacking& state) const { return std::visit(TargetIsPositionVisitor(position), state.target); }
    };

    void Unit::setWeaponTarget(unsigned int weaponIndex, UnitId target)
    {
        auto& weapon = weapons[weaponIndex];
        if (!weapon)
        {
            return;
        }

        if (!std::visit(IsAttackingUnitVisitor(target), weapon->state))
        {
            clearWeaponTarget(weaponIndex);
            weapon->state = UnitWeaponStateAttacking(target);
        }
    }

    void Unit::setWeaponTarget(unsigned int weaponIndex, const SimVector& target)
    {
        auto& weapon = weapons[weaponIndex];
        if (!weapon)
        {
            return;
        }

        if (!std::visit(IsAttackingPositionVisitor(target), weapon->state))
        {
            clearWeaponTarget(weaponIndex);
            weapon->state = UnitWeaponStateAttacking(target);
        }
    }

    void Unit::clearWeaponTarget(unsigned int weaponIndex)
    {
        auto& weapon = weapons[weaponIndex];
        if (!weapon)
        {
            return;
        }

        weapon->state = UnitWeaponStateIdle();
        cobEnvironment->createThread("TargetCleared", {static_cast<int>(weaponIndex)});
    }

    void Unit::clearWeaponTargets()
    {
        for (unsigned int i = 0; i < weapons.size(); ++i)
        {
            clearWeaponTarget(i);
        }
    }

    Matrix4x<SimScalar> Unit::getTransform() const
    {
        return Matrix4x<SimScalar>::translation(position) * Matrix4x<SimScalar>::rotationY(sin(rotation), cos(rotation));
    }

    Matrix4x<SimScalar> Unit::getInverseTransform() const
    {
        return Matrix4x<SimScalar>::rotationY(sin(-rotation), cos(-rotation)) * Matrix4x<SimScalar>::translation(-position);
    }

    bool Unit::isSelectableBy(rwe::PlayerId player) const
    {
        return !isDead() && isOwnedBy(player) && !isBeingBuilt();
    }

    void Unit::activate()
    {
        activated = true;
        cobEnvironment->createThread("Activate");
    }

    void Unit::deactivate()
    {
        activated = false;
        cobEnvironment->createThread("Deactivate");
    }

    MovementClass Unit::getAdHocMovementClass() const
    {
        MovementClass mc;
        mc.minWaterDepth = minWaterDepth;
        mc.maxWaterDepth = maxWaterDepth;
        mc.maxSlope = maxSlope;
        mc.maxWaterSlope = maxWaterSlope;
        mc.footprintX = footprintX;
        mc.footprintZ = footprintZ;
        return mc;
    }

    Metal Unit::getMetalMake() const
    {
        return metalProductionBuffer;
    }

    Energy Unit::getEnergyMake() const
    {
        return energyProductionBuffer;
    }

    Metal Unit::getMetalUse() const
    {
        return previousMetalConsumptionBuffer;
    }

    Energy Unit::getEnergyUse() const
    {
        return previousEnergyConsumptionBuffer;
    }

    void Unit::addMetalDelta(const Metal& metal)
    {
        if (metal >= Metal(0))
        {
            metalProductionBuffer += metal;
        }
        else
        {
            metalConsumptionBuffer -= metal;
        }
    }

    void Unit::addEnergyDelta(const Energy& energy)
    {
        if (energy >= Energy(0))
        {
            energyProductionBuffer += energy;
        }
        else
        {
            energyConsumptionBuffer -= energy;
        }
    }

    void Unit::resetResourceBuffers()
    {
        energyProductionBuffer = Energy(0);
        metalProductionBuffer = Metal(0);
        previousEnergyConsumptionBuffer = energyConsumptionBuffer;
        previousMetalConsumptionBuffer = metalConsumptionBuffer;
        energyConsumptionBuffer = Energy(0);
        metalConsumptionBuffer = Metal(0);
    }

    void Unit::modifyBuildQueue(const std::string& buildUnitType, int count)
    {
        if (count > 0)
        {
            buildQueue.emplace_back(buildUnitType, count);
            return;
        }

        removeFromBuildQueue(buildQueue, buildUnitType, -count);
    }

    std::unordered_map<std::string, int> Unit::getBuildQueueTotals() const
    {
        return getBuildQueueTotalsStatic(buildQueue);
    }

    int Unit::getBuildQueueTotal(const std::string& unitType) const
    {
        int sum = 0;
        for (const auto e : buildQueue)
        {
            if (e.first == unitType)
            {
                sum += e.second;
            }
        }
        return sum;
    }

    std::optional<std::pair<UnitId, SimVector>> Unit::getActiveNanolatheTarget() const
    {
        auto buildingState = std::get_if<BuildingState>(&behaviourState);
        if (buildingState && buildingState->nanoParticleOrigin)
        {
            return std::make_pair(buildingState->targetUnit, *buildingState->nanoParticleOrigin);
        }

        auto factoryBuildingState = std::get_if<FactoryStateBuilding>(&factoryState);
        if (factoryBuildingState && factoryBuildingState->targetUnit && factoryBuildingState->targetUnit->second)
        {
            return std::make_pair(factoryBuildingState->targetUnit->first, *factoryBuildingState->targetUnit->second);
        }

        return std::nullopt;
    }
}
