#include "UnitState.h"
#include <rwe/sim/UnitState_util.h>
#include <rwe/util/Index.h>
#include <rwe/util/match.h>

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

    bool isWater(YardMapCell cell)
    {
        switch (cell)
        {
            case YardMapCell::Water:
            case YardMapCell::WaterPassableWhenOpen:
            case YardMapCell::WaterPassable:
                return true;
            default:
                return false;
        }
    }

    bool isPassable(YardMapCell cell, bool yardMapOpen)
    {
        switch (cell)
        {
            case YardMapCell::GroundPassableWhenOpen:
            case YardMapCell::WaterPassableWhenOpen:
            case YardMapCell::GroundGeoPassableWhenOpen:
                return yardMapOpen;
            case YardMapCell::GroundPassableWhenClosed:
                return !yardMapOpen;
            case YardMapCell::GroundNoFeature:
            case YardMapCell::Geo:
            case YardMapCell::Ground:
            case YardMapCell::Water:
                return false;
            case YardMapCell::GroundPassable:
            case YardMapCell::WaterPassable:
            case YardMapCell::Passable:
                return true;
            default:
                throw std::logic_error("Unknown cell type");
        }
    }

    bool isFlying(const UnitPhysicsInfo& physics)
    {
        return match(
            physics,
            [&](const UnitPhysicsInfoGround&) {
                return false;
            },
            [&](const UnitPhysicsInfoAir&) {
                return true;
            },
            [&](const AirMovementStateTakingOff&) {
                return true;
            },
            [&](const AirMovementStateLanding&) {
                return true;
            });
    }

    SimAngle UnitState::toRotation(const SimVector& direction)
    {
        return atan2(direction.x, direction.z);
    }

    SimVector UnitState::toDirection(SimAngle rotation)
    {
        return SimVector(sin(rotation), 0_ss, cos(rotation));
    }

    std::unordered_map<std::string, int> createPieceIndex(const std::vector<UnitMesh>& pieces)
    {
        std::unordered_map<std::string, int> m;
        for (Index i = 0; i < getSize(pieces); ++i)
        {
            m.insert_or_assign(toUpper(pieces[i].name), i);
        }
        return m;
    }

    UnitState::UnitState(const std::vector<UnitMesh>& pieces, std::unique_ptr<CobEnvironment>&& cobEnvironment)
        : pieces(pieces), pieceNameToIndices(createPieceIndex(this->pieces)), cobEnvironment(std::move(cobEnvironment))
    {
    }

    bool UnitState::isBeingBuilt(const UnitDefinition& unitDefinition) const
    {
        return buildTimeCompleted < unitDefinition.buildTime;
    }

    unsigned int UnitState::getBuildPercentLeft(const UnitDefinition& unitDefinition) const
    {
        return 100u - ((buildTimeCompleted * 100u) / unitDefinition.buildTime);
    }

    float UnitState::getPreciseCompletePercent(const UnitDefinition& unitDefinition) const
    {
        return static_cast<float>(buildTimeCompleted) / static_cast<float>(unitDefinition.buildTime);
    }

    UnitState::BuildCostInfo UnitState::getBuildCostInfo(const UnitDefinition& unitDefinition, unsigned int buildTimeContribution)
    {
        auto remainingBuildTime = unitDefinition.buildTime - buildTimeCompleted;
        if (buildTimeContribution > remainingBuildTime)
        {
            buildTimeContribution = remainingBuildTime;
        }

        auto oldProgressEnergy = Energy((buildTimeCompleted * unitDefinition.buildCostEnergy.value) / unitDefinition.buildTime);
        auto oldProgressMetal = Metal((buildTimeCompleted * unitDefinition.buildCostMetal.value) / unitDefinition.buildTime);

        auto newBuildTimeCompleted = buildTimeCompleted + buildTimeContribution;

        auto newProgressEnergy = Energy((newBuildTimeCompleted * unitDefinition.buildCostEnergy.value) / unitDefinition.buildTime);
        auto newProgressMetal = Metal((newBuildTimeCompleted * unitDefinition.buildCostMetal.value) / unitDefinition.buildTime);

        auto deltaEnergy = newProgressEnergy - oldProgressEnergy;
        auto deltaMetal = newProgressMetal - oldProgressMetal;

        return BuildCostInfo{buildTimeContribution, deltaEnergy, deltaMetal};
    }

    bool UnitState::addBuildProgress(const UnitDefinition& unitDefinition, unsigned int buildTimeContribution)
    {
        auto remainingBuildTime = unitDefinition.buildTime - buildTimeCompleted;
        if (buildTimeContribution > remainingBuildTime)
        {
            buildTimeContribution = remainingBuildTime;
        }

        auto oldProgressHp = (buildTimeCompleted * unitDefinition.maxHitPoints) / unitDefinition.buildTime;

        buildTimeCompleted += buildTimeContribution;

        auto newProgressHp = (buildTimeCompleted * unitDefinition.maxHitPoints) / unitDefinition.buildTime;

        auto deltaHp = newProgressHp - oldProgressHp;

        // add HP up to the maximum
        if (hitPoints + deltaHp >= unitDefinition.maxHitPoints)
        {
            hitPoints = unitDefinition.maxHitPoints;
        }
        else
        {
            hitPoints += deltaHp;
        }

        return buildTimeCompleted == unitDefinition.buildTime;
    }

    void UnitState::moveObject(const std::string& pieceName, SimAxis axis, SimScalar targetPosition, SimScalar speed)
    {
        auto piece = findPiece(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        UnitMesh::MoveOperation op(targetPosition, speed);

        switch (axis)
        {
            case SimAxis::X:
                piece->get().xMoveOperation = op;
                break;
            case SimAxis::Y:
                piece->get().yMoveOperation = op;
                break;
            case SimAxis::Z:
                piece->get().zMoveOperation = op;
                break;
        }
    }

    void UnitState::moveObjectNow(const std::string& pieceName, SimAxis axis, SimScalar targetPosition)
    {
        auto piece = findPiece(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        switch (axis)
        {
            case SimAxis::X:
                piece->get().offset.x = targetPosition;
                piece->get().xMoveOperation = std::nullopt;
                break;
            case SimAxis::Y:
                piece->get().offset.y = targetPosition;
                piece->get().yMoveOperation = std::nullopt;
                break;
            case SimAxis::Z:
                piece->get().offset.z = targetPosition;
                piece->get().zMoveOperation = std::nullopt;
                break;
        }
    }

    void UnitState::turnObject(const std::string& pieceName, SimAxis axis, SimAngle targetAngle, SimScalar speed)
    {
        auto piece = findPiece(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        UnitMesh::TurnOperation op(targetAngle, speed);

        switch (axis)
        {
            case SimAxis::X:
                piece->get().xTurnOperation = op;
                break;
            case SimAxis::Y:
                piece->get().yTurnOperation = op;
                break;
            case SimAxis::Z:
                piece->get().zTurnOperation = op;
                break;
        }
    }

    void UnitState::turnObjectNow(const std::string& pieceName, SimAxis axis, SimAngle targetAngle)
    {
        auto piece = findPiece(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        switch (axis)
        {
            case SimAxis::X:
                piece->get().rotationX = targetAngle;
                piece->get().xTurnOperation = std::nullopt;
                break;
            case SimAxis::Y:
                piece->get().rotationY = targetAngle;
                piece->get().yTurnOperation = std::nullopt;
                break;
            case SimAxis::Z:
                piece->get().rotationZ = targetAngle;
                piece->get().zTurnOperation = std::nullopt;
                break;
        }
    }

    void UnitState::spinObject(const std::string& pieceName, SimAxis axis, SimScalar speed, SimScalar acceleration)
    {
        auto piece = findPiece(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        UnitMesh::SpinOperation op(acceleration == 0_ss ? speed : 0_ss, speed, acceleration);

        switch (axis)
        {
            case SimAxis::X:
                piece->get().xTurnOperation = op;
                break;
            case SimAxis::Y:
                piece->get().yTurnOperation = op;
                break;
            case SimAxis::Z:
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

    void UnitState::stopSpinObject(const std::string& pieceName, SimAxis axis, SimScalar deceleration)
    {

        auto piece = findPiece(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        switch (axis)
        {
            case SimAxis::X:
                setStopSpinOp(piece->get().xTurnOperation, deceleration);
                break;
            case SimAxis::Y:
                setStopSpinOp(piece->get().yTurnOperation, deceleration);
                break;
            case SimAxis::Z:
                setStopSpinOp(piece->get().zTurnOperation, deceleration);
                break;
        }
    }

    bool UnitState::isMoveInProgress(const std::string& pieceName, SimAxis axis) const
    {
        auto piece = findPiece(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name " + pieceName);
        }

        switch (axis)
        {
            case SimAxis::X:
                return !!(piece->get().xMoveOperation);
            case SimAxis::Y:
                return !!(piece->get().yMoveOperation);
            case SimAxis::Z:
                return !!(piece->get().zMoveOperation);
        }

        throw std::logic_error("Invalid axis");
    }

    bool UnitState::isTurnInProgress(const std::string& pieceName, SimAxis axis) const
    {
        auto piece = findPiece(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name " + pieceName);
        }

        switch (axis)
        {
            case SimAxis::X:
                return !!(piece->get().xTurnOperation);
            case SimAxis::Y:
                return !!(piece->get().yTurnOperation);
            case SimAxis::Z:
                return !!(piece->get().zTurnOperation);
        }

        throw std::logic_error("Invalid axis");
    }

    bool UnitState::isOwnedBy(PlayerId playerId) const
    {
        return owner == playerId;
    }

    bool UnitState::isAlive() const
    {
        return std::holds_alternative<LifeStateAlive>(lifeState);
    }

    bool UnitState::isDead() const
    {
        return std::holds_alternative<LifeStateDead>(lifeState);
    }

    void UnitState::markAsDead()
    {
        lifeState = LifeStateDead{true};
    }

    void UnitState::markAsDeadNoCorpse()
    {
        lifeState = LifeStateDead{false};
    }

    void UnitState::finishBuilding(const UnitDefinition& unitDefinition)
    {
        // FIXME: ignores damage taken during building
        hitPoints = unitDefinition.maxHitPoints;
        buildTimeCompleted = unitDefinition.buildTime;
    }

    void UnitState::clearOrders()
    {
        orders.clear();
        buildOrderUnitId = std::nullopt;

        // not clear if this really belongs here
        clearWeaponTargets();
    }

    void UnitState::replaceOrders(const std::deque<UnitOrder>& newOrders)
    {
        orders = newOrders;
    }

    void UnitState::addOrder(const UnitOrder& order)
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

    void UnitState::setWeaponTarget(unsigned int weaponIndex, UnitId target)
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

    void UnitState::setWeaponTarget(unsigned int weaponIndex, const SimVector& target)
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

    void UnitState::clearWeaponTarget(unsigned int weaponIndex)
    {
        auto& weapon = weapons[weaponIndex];
        if (!weapon)
        {
            return;
        }

        weapon->state = UnitWeaponStateIdle();
        cobEnvironment->createThread("TargetCleared", {static_cast<int>(weaponIndex)});
    }

    void UnitState::clearWeaponTargets()
    {
        for (Index i = 0; i < getSize(weapons); ++i)
        {
            clearWeaponTarget(i);
        }
    }

    Matrix4x<SimScalar> UnitState::getTransform() const
    {
        return Matrix4x<SimScalar>::translation(position) * Matrix4x<SimScalar>::rotationY(sin(rotation), cos(rotation));
    }

    Matrix4x<SimScalar> UnitState::getInverseTransform() const
    {
        return Matrix4x<SimScalar>::rotationY(sin(-rotation), cos(-rotation)) * Matrix4x<SimScalar>::translation(-position);
    }

    bool UnitState::isSelectableBy(const UnitDefinition& unitDefinition, rwe::PlayerId player) const
    {
        return !isDead() && isOwnedBy(player) && !isBeingBuilt(unitDefinition);
    }

    void UnitState::activate()
    {
        activated = true;
        cobEnvironment->createThread("Activate");
    }

    void UnitState::deactivate()
    {
        activated = false;
        cobEnvironment->createThread("Deactivate");
    }

    Metal UnitState::getMetalMake() const
    {
        return metalProductionBuffer;
    }

    Energy UnitState::getEnergyMake() const
    {
        return energyProductionBuffer;
    }

    Metal UnitState::getMetalUse() const
    {
        return previousMetalConsumptionBuffer;
    }

    Energy UnitState::getEnergyUse() const
    {
        return previousEnergyConsumptionBuffer;
    }

    void UnitState::addMetalDelta(const Metal& metal)
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

    void UnitState::addEnergyDelta(const Energy& energy)
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

    void UnitState::resetResourceBuffers()
    {
        energyProductionBuffer = Energy(0);
        metalProductionBuffer = Metal(0);
        previousEnergyConsumptionBuffer = energyConsumptionBuffer;
        previousMetalConsumptionBuffer = metalConsumptionBuffer;
        energyConsumptionBuffer = Energy(0);
        metalConsumptionBuffer = Metal(0);
    }

    void UnitState::modifyBuildQueue(const std::string& buildUnitType, int count)
    {
        if (count > 0)
        {
            buildQueue.emplace_back(buildUnitType, count);
            return;
        }

        removeFromBuildQueue(buildQueue, buildUnitType, -count);
    }

    std::unordered_map<std::string, int> UnitState::getBuildQueueTotals() const
    {
        return getBuildQueueTotalsStatic(buildQueue);
    }

    int UnitState::getBuildQueueTotal(const std::string& unitType) const
    {
        int sum = 0;
        for (const auto& e : buildQueue)
        {
            if (e.first == unitType)
            {
                sum += e.second;
            }
        }
        return sum;
    }

    std::optional<std::pair<UnitId, SimVector>> UnitState::getActiveNanolatheTarget() const
    {
        auto buildingState = std::get_if<UnitBehaviorStateBuilding>(&behaviourState);
        if (buildingState && buildingState->nanoParticleOrigin)
        {
            return std::make_pair(buildingState->targetUnit, *buildingState->nanoParticleOrigin);
        }

        auto factoryBuildingState = std::get_if<FactoryBehaviorStateBuilding>(&factoryState);
        if (factoryBuildingState && factoryBuildingState->targetUnit && factoryBuildingState->targetUnit->second)
        {
            return std::make_pair(factoryBuildingState->targetUnit->first, *factoryBuildingState->targetUnit->second);
        }

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const UnitMesh>> UnitState::findPiece(const std::string& pieceName) const
    {
        auto pieceIndexIt = pieceNameToIndices.find(toUpper(pieceName));
        if (pieceIndexIt == pieceNameToIndices.end())
        {
            return std::nullopt;
        }

        return pieces[pieceIndexIt->second];
    }

    std::optional<std::reference_wrapper<UnitMesh>> UnitState::findPiece(const std::string& pieceName)
    {
        auto pieceIndexIt = pieceNameToIndices.find(toUpper(pieceName));
        if (pieceIndexIt == pieceNameToIndices.end())
        {
            return std::nullopt;
        }

        return pieces[pieceIndexIt->second];
    }
}
