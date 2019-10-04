#include "UnitBehaviorService.h"
#include <rwe/GameScene.h>
#include <rwe/cob/CobExecutionContext.h>
#include <rwe/geometry/Circle2x.h>
#include <rwe/math/rwe_math.h>
#include <rwe/movement.h>
#include <rwe/overloaded.h>

namespace rwe
{
    Vector2x<SimScalar> Vector2xFromLengthAndAngle(SimScalar length, SimAngle angle)
    {
        return Vector2x<SimScalar>(sin(angle), cos(angle)) * length;
    }

    bool isWithinTurningCircle(const SimVector& dest, SimScalar speed, SimScalar turnRate, SimAngle currentDirection)
    {
        auto turnRadius = speed / turnRate;

        auto anticlockwiseCircleAngle = currentDirection + QuarterTurn;
        auto clockwiseCircleAngle = currentDirection - QuarterTurn;
        auto anticlockwiseCircle = Circle2x<SimScalar>(turnRadius, Vector2xFromLengthAndAngle(turnRadius, anticlockwiseCircleAngle));
        auto clockwiseCircle = Circle2x<SimScalar>(turnRadius, Vector2xFromLengthAndAngle(turnRadius, clockwiseCircleAngle));

        return anticlockwiseCircle.contains(dest.xz()) || clockwiseCircle.contains(dest.xz());
    }

    UnitBehaviorService::UnitBehaviorService(
        GameScene* scene,
        UnitFactory* unitFactory)
        : scene(scene), unitFactory(unitFactory)
    {
    }

    void UnitBehaviorService::update(UnitId unitId)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        auto previousSpeed = unit.currentSpeed;

        // Clear steering targets.
        unit.targetAngle = unit.rotation;
        unit.targetSpeed = 0_ss;

        // Run unit and weapon AI
        if (!unit.isBeingBuilt())
        {
            // check our build queue
            if (!unit.buildQueue.empty())
            {
                auto& entry = unit.buildQueue.front();
                if (handleBuild(unitId, entry.first))
                {
                    if (entry.second > 1)
                    {
                        --entry.second;
                    }
                    else
                    {
                        unit.buildQueue.pop_front();
                    }
                }
            }
            else
            {
                clearBuild(unitId);
            }

            // check our orders
            if (!unit.orders.empty())
            {
                const auto& order = unit.orders.front();

                // process move orders
                if (handleOrder(unitId, order))
                {
                    unit.orders.pop_front();
                }
            }

            for (unsigned int i = 0; i < unit.weapons.size(); ++i)
            {
                updateWeapon(unitId, i);
            }
        }

        if (unit.isMobile)
        {
            applyUnitSteering(unitId);

            if (unit.currentSpeed > 0_ss && previousSpeed == 0_ss)
            {
                unit.cobEnvironment->createThread("StartMoving");
            }
            else if (unit.currentSpeed == 0_ss && previousSpeed > 0_ss)
            {
                unit.cobEnvironment->createThread("StopMoving");
            }

            updateUnitPosition(unitId);
        }
    }

    std::pair<SimAngle, SimAngle> UnitBehaviorService::computeHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to)
    {
        auto aimVector = to - from;
        if (aimVector.lengthSquared() == 0_ss)
        {
            aimVector = Unit::toDirection(rotation);
        }

        SimVector aimVectorXZ(aimVector.x, 0_ss, aimVector.z);

        auto heading = Unit::toRotation(aimVectorXZ);
        heading = heading - rotation;

        auto pitch = atan2(aimVector.y, aimVectorXZ.length());

        return {heading, pitch};
    }

    bool UnitBehaviorService::followPath(Unit& unit, PathFollowingInfo& path)
    {
        const auto& destination = *path.currentWaypoint;
        SimVector xzPosition(unit.position.x, 0_ss, unit.position.z);
        SimVector xzDestination(destination.x, 0_ss, destination.z);
        auto distanceSquared = xzPosition.distanceSquared(xzDestination);

        auto isFinalDestination = path.currentWaypoint == (path.path.waypoints.end() - 1);

        if (distanceSquared < (8_ss * 8_ss))
        {
            if (isFinalDestination)
            {
                return true;
            }
            else
            {
                ++path.currentWaypoint;
            }
        }
        else
        {
            // steer towards the goal
            auto xzDirection = xzDestination - xzPosition;
            unit.targetAngle = Unit::toRotation(xzDirection);

            // drive at full speed until we need to brake
            // to turn or to arrive at the goal
            auto brakingDistance = (unit.currentSpeed * unit.currentSpeed) / (2_ss * unit.brakeRate);

            if (isWithinTurningCircle(xzDirection, unit.currentSpeed, unit.turnRate, unit.rotation))
            {
                unit.targetSpeed = 0_ss;
            }
            else if (isFinalDestination && distanceSquared <= (brakingDistance * brakingDistance))
            {
                unit.targetSpeed = 0_ss;
            }
            else
            {
                unit.targetSpeed = unit.maxSpeed;
            }
        }

        return false;
    }

    void UnitBehaviorService::updateWeapon(UnitId id, unsigned int weaponIndex)
    {
        auto& unit = scene->getSimulation().getUnit(id);
        auto& weapon = unit.weapons[weaponIndex];
        if (!weapon)
        {
            return;
        }

        if (auto idleState = std::get_if<UnitWeaponStateIdle>(&weapon->state); idleState != nullptr)
        {
            // attempt to acquire a target
            if (!weapon->commandFire && unit.fireOrders == UnitFireOrders::FireAtWill)
            {
                for (const auto& entry : scene->getSimulation().units)
                {
                    auto otherUnitId = entry.first;
                    const auto& otherUnit = entry.second;

                    if (otherUnit.isDead())
                    {
                        continue;
                    }

                    if (otherUnit.isOwnedBy(unit.owner))
                    {
                        continue;
                    }

                    if (unit.position.distanceSquared(otherUnit.position) > weapon->maxRange * weapon->maxRange)
                    {
                        continue;
                    }

                    weapon->state = UnitWeaponStateAttacking(otherUnitId);
                    break;
                }
            }
        }
        else if (auto aimingState = std::get_if<UnitWeaponStateAttacking>(&weapon->state); aimingState != nullptr)
        {
            // If we are not fire-at-will, the target is a unit,
            // and we don't have an explicit order to attack that unit,
            // drop the target.
            // This can happen if we acquired the target ourselves while in fire-at-will,
            // but then the player switched us to another firing mode.
            if (unit.fireOrders != UnitFireOrders::FireAtWill)
            {
                if (auto targetUnit = std::get_if<UnitId>(&aimingState->target); targetUnit != nullptr)
                {
                    if (unit.orders.empty())
                    {
                        unit.clearWeaponTarget(weaponIndex);
                        return;
                    }
                    else if (auto attackOrder = std::get_if<AttackOrder>(&unit.orders.front()); attackOrder != nullptr)
                    {
                        if (auto attackTarget = std::get_if<UnitId>(&attackOrder->target); attackTarget == nullptr || *attackTarget != *targetUnit)
                        {
                            unit.clearWeaponTarget(weaponIndex);
                            return;
                        }
                    }
                }
            }

            auto targetPosition = getTargetPosition(aimingState->target);

            if (!targetPosition || unit.position.distanceSquared(*targetPosition) > weapon->maxRange * weapon->maxRange)
            {
                unit.clearWeaponTarget(weaponIndex);
            }
            else if (!aimingState->aimInfo)
            {
                auto aimFromPosition = getAimingPoint(id, weaponIndex);

                auto headingAndPitch = computeHeadingAndPitch(unit.rotation, aimFromPosition, *targetPosition);
                auto heading = headingAndPitch.first;
                auto pitch = headingAndPitch.second;

                auto threadId = unit.cobEnvironment->createThread(getAimScriptName(weaponIndex), {toCobAngle(heading).value, toCobAngle(pitch).value});

                if (threadId)
                {
                    aimingState->aimInfo = UnitWeaponStateAttacking::AimInfo{*threadId, heading, pitch};
                }
                else
                {
                    // We couldn't launch an aiming script (there isn't one),
                    // just go straight to firing.
                    tryFireWeapon(id, weaponIndex, *targetPosition);
                }
            }
            else
            {
                const auto aimInfo = *aimingState->aimInfo;
                auto returnValue = unit.cobEnvironment->tryReapThread(aimInfo.thread);
                if (returnValue)
                {
                    // we successfully reaped, clear the thread.
                    aimingState->aimInfo = std::nullopt;

                    if (*returnValue)
                    {
                        // aiming was successful, check the target again for drift
                        auto targetPosition = getTargetPosition(aimingState->target);
                        auto aimFromPosition = getAimingPoint(id, weaponIndex);

                        auto headingAndPitch = computeHeadingAndPitch(unit.rotation, aimFromPosition, *targetPosition);
                        auto heading = headingAndPitch.first;
                        auto pitch = headingAndPitch.second;

                        // if the target is close enough, try to fire
                        if (angleBetweenIsLessOrEqual(heading, aimInfo.lastHeading, weapon->tolerance) && angleBetweenIsLessOrEqual(pitch, aimInfo.lastPitch, weapon->pitchTolerance))
                        {
                            tryFireWeapon(id, weaponIndex, *targetPosition);
                        }
                    }
                }
            }
        }
    }

    void UnitBehaviorService::tryFireWeapon(UnitId id, unsigned int weaponIndex, const SimVector& targetPosition)
    {
        auto& unit = scene->getSimulation().getUnit(id);
        auto& weapon = unit.weapons[weaponIndex];

        if (!weapon)
        {
            return;
        }

        // wait for the weapon to reload
        auto gameTime = scene->getGameTime();
        if (gameTime < weapon->readyTime)
        {
            return;
        }

        // spawn a projectile from the firing point
        auto firingPoint = getFiringPoint(id, weaponIndex);
        auto targetVector = targetPosition - firingPoint;
        if (weapon->startSmoke)
        {
            scene->createLightSmoke(firingPoint);
        }
        scene->getSimulation().spawnLaser(unit.owner, *weapon, firingPoint, targetVector.normalized());

        if (weapon->soundStart)
        {
            scene->playUnitSound(id, *weapon->soundStart);
        }
        unit.cobEnvironment->createThread(getFireScriptName(weaponIndex));

        // we are reloading now
        weapon->readyTime = gameTime + deltaSecondsToTicks(weapon->reloadTime);
    }

    void UnitBehaviorService::applyUnitSteering(UnitId id)
    {
        updateUnitRotation(id);
        updateUnitSpeed(id);
    }

    void UnitBehaviorService::updateUnitRotation(UnitId id)
    {
        auto& unit = scene->getSimulation().getUnit(id);
        auto turnRateThisFrame = SimAngle(unit.turnRate.value);
        unit.rotation = turnTowards(unit.rotation, unit.targetAngle, turnRateThisFrame);
    }

    void UnitBehaviorService::updateUnitSpeed(UnitId id)
    {
        auto& unit = scene->getSimulation().getUnit(id);

        if (unit.targetSpeed > unit.currentSpeed)
        {
            // accelerate to target speed
            if (unit.targetSpeed - unit.currentSpeed <= unit.acceleration)
            {
                unit.currentSpeed = unit.targetSpeed;
            }
            else
            {
                unit.currentSpeed += unit.acceleration;
            }
        }
        else
        {
            // brake to target speed
            if (unit.currentSpeed - unit.targetSpeed <= unit.brakeRate)
            {
                unit.currentSpeed = unit.targetSpeed;
            }
            else
            {
                unit.currentSpeed -= unit.brakeRate;
            }
        }

        auto effectiveMaxSpeed = unit.maxSpeed;
        if (unit.position.y < scene->getTerrain().getSeaLevel())
        {
            effectiveMaxSpeed /= 2_ss;
        }
        unit.currentSpeed = std::clamp(unit.currentSpeed, 0_ss, effectiveMaxSpeed);
    }

    void UnitBehaviorService::updateUnitPosition(UnitId unitId)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        auto direction = Unit::toDirection(unit.rotation);

        unit.inCollision = false;

        if (unit.currentSpeed > 0_ss)
        {
            auto newPosition = unit.position + (direction * unit.currentSpeed);
            newPosition.y = scene->getTerrain().getHeightAt(newPosition.x, newPosition.z);

            if (!tryApplyMovementToPosition(unitId, newPosition))
            {
                unit.inCollision = true;

                // if we failed to move, try in each axis separately
                // to see if we can complete a "partial" movement
                const SimVector maskX(0_ss, 1_ss, 1_ss);
                const SimVector maskZ(1_ss, 1_ss, 0_ss);

                SimVector newPos1;
                SimVector newPos2;
                if (direction.x > direction.z)
                {
                    newPos1 = unit.position + (direction * maskZ * unit.currentSpeed);
                    newPos2 = unit.position + (direction * maskX * unit.currentSpeed);
                }
                else
                {
                    newPos1 = unit.position + (direction * maskX * unit.currentSpeed);
                    newPos2 = unit.position + (direction * maskZ * unit.currentSpeed);
                }
                newPos1.y = scene->getTerrain().getHeightAt(newPos1.x, newPos1.z);
                newPos2.y = scene->getTerrain().getHeightAt(newPos2.x, newPos2.z);

                if (!tryApplyMovementToPosition(unitId, newPos1))
                {
                    tryApplyMovementToPosition(unitId, newPos2);
                }
            }
        }
    }

    bool UnitBehaviorService::tryApplyMovementToPosition(UnitId id, const SimVector& newPosition)
    {
        auto& sim = scene->getSimulation();
        auto& unit = sim.getUnit(id);

        // check for collision at the new position
        auto newFootprintRegion = scene->computeFootprintRegion(newPosition, unit.footprintX, unit.footprintZ);

        // Unlike for pathfinding, TA doesn't care about the unit's actual movement class for collision checks,
        // it only cares about the attributes defined directly on the unit.
        // Jam these into an ad-hoc movement class to pass into our walkability check.
        if (!isGridPointWalkable(sim.terrain, unit.getAdHocMovementClass(), newFootprintRegion.x, newFootprintRegion.y))
        {
            return false;
        }

        if (scene->isCollisionAt(newFootprintRegion, id))
        {
            return false;
        }

        // we passed all collision checks, update accordingly
        auto footprintRegion = scene->computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        scene->moveUnitOccupiedArea(footprintRegion, newFootprintRegion, id);
        unit.position = newPosition;
        return true;
    }

    std::string UnitBehaviorService::getAimScriptName(unsigned int weaponIndex) const
    {
        switch (weaponIndex)
        {
            case 0:
                return "AimPrimary";
            case 1:
                return "AimSecondary";
            case 2:
                return "AimTertiary";
            default:
                throw std::logic_error("Invalid wepaon index: " + std::to_string(weaponIndex));
        }
    }

    std::string UnitBehaviorService::getAimFromScriptName(unsigned int weaponIndex) const
    {
        switch (weaponIndex)
        {
            case 0:
                return "AimFromPrimary";
            case 1:
                return "AimFromSecondary";
            case 2:
                return "AimFromTertiary";
            default:
                throw std::logic_error("Invalid weapon index: " + std::to_string(weaponIndex));
        }
    }

    std::string UnitBehaviorService::getFireScriptName(unsigned int weaponIndex) const
    {
        switch (weaponIndex)
        {
            case 0:
                return "FirePrimary";
            case 1:
                return "FireSecondary";
            case 2:
                return "FireTertiary";
            default:
                throw std::logic_error("Invalid weapon index: " + std::to_string(weaponIndex));
        }
    }

    std::string UnitBehaviorService::getQueryScriptName(unsigned int weaponIndex) const
    {
        switch (weaponIndex)
        {
            case 0:
                return "QueryPrimary";
            case 1:
                return "QuerySecondary";
            case 2:
                return "QueryTertiary";
            default:
                throw std::logic_error("Invalid wepaon index: " + std::to_string(weaponIndex));
        }
    }

    std::optional<int> UnitBehaviorService::runCobQuery(UnitId id, const std::string& name)
    {
        auto& unit = scene->getSimulation().getUnit(id);
        auto thread = unit.cobEnvironment->createNonScheduledThread(name, {0});
        if (!thread)
        {
            return std::nullopt;
        }
        CobExecutionContext context(scene, &scene->getSimulation(), unit.cobEnvironment.get(), &*thread, id);
        auto status = context.execute();
        if (std::get_if<CobEnvironment::FinishedStatus>(&status) == nullptr)
        {
            throw std::runtime_error("Synchronous cob query thread blocked before completion");
        }

        auto result = thread->returnLocals[0];
        return result;
    }

    SimVector UnitBehaviorService::getAimingPoint(UnitId id, unsigned int weaponIndex)
    {
        auto scriptName = getAimFromScriptName(weaponIndex);
        auto pieceId = runCobQuery(id, scriptName);
        if (!pieceId)
        {
            return getFiringPoint(id, weaponIndex);
        }

        return getPiecePosition(id, *pieceId);
    }

    SimVector UnitBehaviorService::getFiringPoint(UnitId id, unsigned int weaponIndex)
    {

        auto scriptName = getQueryScriptName(weaponIndex);
        auto pieceId = runCobQuery(id, scriptName);
        if (!pieceId)
        {
            return scene->getSimulation().getUnit(id).position;
        }

        return getPiecePosition(id, *pieceId);
    }

    SimVector UnitBehaviorService::getSweetSpot(UnitId id)
    {
        auto pieceId = runCobQuery(id, "SweetSpot");
        if (!pieceId)
        {
            return scene->getSimulation().getUnit(id).position;
        }

        return getPiecePosition(id, *pieceId);
    }

    std::optional<SimVector> UnitBehaviorService::tryGetSweetSpot(UnitId id)
    {
        if (!scene->getSimulation().unitExists(id))
        {
            return std::nullopt;
        }

        return getSweetSpot(id);
    }

    bool UnitBehaviorService::handleOrder(UnitId unitId, const UnitOrder& order)
    {
        return match(
            order,
            [this, unitId](const MoveOrder& o) {
                return handleMoveOrder(unitId, o);
            },
            [this, unitId](const AttackOrder& o) {
                return handleAttackOrder(unitId, o);
            },
            [this, unitId](const BuildOrder& o) {
                return handleBuildOrder(unitId, o);
            },
            [this, unitId](const BuggerOffOrder& o) {
                return handleBuggerOffOrder(unitId, o);
            });
    }

    bool UnitBehaviorService::handleMoveOrder(UnitId unitId, const MoveOrder& moveOrder)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        if (auto idleState = std::get_if<IdleState>(&unit.behaviourState); idleState != nullptr)
        {
            // request a path to follow
            scene->getSimulation().requestPath(unitId);
            const auto& destination = moveOrder.destination;
            unit.behaviourState = MovingState{destination, std::nullopt, true};
        }
        else if (auto movingState = std::get_if<MovingState>(&unit.behaviourState); movingState != nullptr)
        {
            // if we are colliding, request a new path
            if (unit.inCollision && !movingState->pathRequested)
            {
                auto& sim = scene->getSimulation();

                // only request a new path if we don't have one yet,
                // or we've already had our current one for a bit
                if (!movingState->path || (sim.gameTime - movingState->path->pathCreationTime) >= GameTime(60))
                {
                    sim.requestPath(unitId);
                    movingState->pathRequested = true;
                }
            }

            // if a path is available, attempt to follow it
            auto& pathToFollow = movingState->path;
            if (pathToFollow)
            {
                if (followPath(unit, *pathToFollow))
                {
                    // we finished following the path,
                    // order complete
                    unit.behaviourState = IdleState();

                    if (unit.arrivedSound)
                    {
                        scene->playSoundOnSelectChannel(*unit.arrivedSound);
                    }

                    return true;
                }
            }
        }

        return false;
    }

    bool UnitBehaviorService::handleAttackOrder(UnitId unitId, const AttackOrder& attackOrder)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        if (!unit.weapons[0])
        {
            return true;
        }
        else
        {
            auto targetPosition = getTargetPosition(attackOrder.target);
            if (!targetPosition)
            {
                // target has gone away, throw away this order
                return true;
            }

            auto maxRangeSquared = unit.weapons[0]->maxRange * unit.weapons[0]->maxRange;
            if (auto idleState = std::get_if<IdleState>(&unit.behaviourState); idleState != nullptr)
            {
                // if we're out of range, drive into range
                if (unit.position.distanceSquared(*targetPosition) > maxRangeSquared)
                {
                    // request a path to follow
                    scene->getSimulation().requestPath(unitId);
                    auto destination = attackTargetToMovingStateGoal(attackOrder.target);
                    unit.behaviourState = MovingState{destination, std::nullopt, true};
                }
                else
                {
                    // we're in range, aim weapons
                    for (unsigned int i = 0; i < 2; ++i)
                    {
                        unit.setWeaponTarget(i, *targetPosition);
                    }
                }
            }
            else if (auto movingState = std::get_if<MovingState>(&unit.behaviourState); movingState != nullptr)
            {
                if (unit.position.distanceSquared(*targetPosition) <= maxRangeSquared)
                {
                    unit.behaviourState = IdleState();
                }
                else
                {
                    // TODO: consider requesting a new path if the target unit has moved significantly

                    // if we are colliding, request a new path
                    if (unit.inCollision && !movingState->pathRequested)
                    {
                        auto& sim = scene->getSimulation();

                        // only request a new path if we don't have one yet,
                        // or we've already had our current one for a bit
                        if (!movingState->path || (sim.gameTime - movingState->path->pathCreationTime) >= GameTime(60))
                        {
                            sim.requestPath(unitId);
                            movingState->pathRequested = true;
                        }
                    }

                    // if a path is available, attempt to follow it
                    auto& pathToFollow = movingState->path;
                    if (pathToFollow)
                    {
                        if (followPath(unit, *pathToFollow))
                        {
                            // we finished following the path,
                            // go back to idle
                            unit.behaviourState = IdleState();
                        }
                    }
                }
            }
        }

        return false;
    }

    bool UnitBehaviorService::handleBuildOrder(UnitId unitId, const BuildOrder& buildOrder)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        if (auto idleState = std::get_if<IdleState>(&unit.behaviourState); idleState != nullptr)
        {
            // request a path to get to the build site
            auto footprint = unitFactory->getUnitFootprint(buildOrder.unitType);
            auto footprintRect = scene->computeFootprintRegion(buildOrder.position, footprint.x, footprint.y);
            scene->getSimulation().requestPath(unitId);
            unit.behaviourState = MovingState{footprintRect, std::nullopt, true};
        }
        else if (auto movingState = std::get_if<MovingState>(&unit.behaviourState); movingState != nullptr)
        {
            // if we are colliding, request a new path
            if (unit.inCollision && !movingState->pathRequested)
            {
                auto& sim = scene->getSimulation();

                // only request a new path if we don't have one yet,
                // or we've already had our current one for a bit
                if (!movingState->path || (sim.gameTime - movingState->path->pathCreationTime) >= GameTime(60))
                {
                    sim.requestPath(unitId);
                    movingState->pathRequested = true;
                }
            }

            // if a path is available, attempt to follow it
            auto& pathToFollow = movingState->path;
            if (pathToFollow)
            {
                if (followPath(unit, *pathToFollow))
                {
                    auto targetUnitId = scene->spawnUnit(buildOrder.unitType, unit.owner, buildOrder.position);
                    if (!targetUnitId)
                    {
                        // we failed to create the unit -- give up
                        unit.behaviourState = IdleState();
                        return true;
                    }

                    if (unit.buildSound)
                    {
                        scene->playSoundOnSelectChannel(*unit.buildSound);
                    }

                    auto nanoFromPosition = getNanoPoint(unitId);
                    auto headingAndPitch = computeHeadingAndPitch(unit.rotation, nanoFromPosition, buildOrder.position);
                    auto heading = headingAndPitch.first;
                    auto pitch = headingAndPitch.second;

                    unit.cobEnvironment->createThread("StartBuilding", {toCobAngle(heading).value, toCobAngle(pitch).value});
                    unit.behaviourState = BuildingState{*targetUnitId};
                }
            }
        }
        else if (auto buildingState = std::get_if<BuildingState>(&unit.behaviourState); buildingState != nullptr)
        {
            if (!scene->getSimulation().unitExists(buildingState->targetUnit))
            {
                // the unit has gone away (maybe it was killed?), give up
                unit.cobEnvironment->createThread("StopBuilding");
                unit.behaviourState = IdleState();
                return true;
            }

            auto& targetUnit = scene->getSimulation().getUnit(buildingState->targetUnit);
            if (targetUnit.isDead())
            {
                // the target is dead, give up
                unit.cobEnvironment->createThread("StopBuilding");
                unit.behaviourState = IdleState();
                return true;
            }

            if (!targetUnit.isBeingBuilt())
            {
                // the target does not need to be built anymore.
                // Probably because it's finished -- we did it!
                unit.cobEnvironment->createThread("StopBuilding");
                unit.behaviourState = IdleState();
                return true;
            }

            if (!unit.inBuildStance)
            {
                // We are not in the correct stance to build the unit yet, wait.
                return false;
            }

            auto& sim = scene->getSimulation();
            auto costs = targetUnit.getBuildCostInfo(unit.workerTimePerTick);
            auto gotResources = sim.addResourceDelta(
                unitId,
                -Energy(targetUnit.energyCost.value * static_cast<float>(unit.workerTimePerTick) / static_cast<float>(targetUnit.buildTime)),
                -Metal(targetUnit.metalCost.value * static_cast<float>(unit.workerTimePerTick) / static_cast<float>(targetUnit.buildTime)),
                -costs.energyCost,
                -costs.metalCost);

            if (!gotResources)
            {
                // we don't have resources available to build -- wait
                return false;
            }

            if (targetUnit.addBuildProgress(unit.workerTimePerTick))
            {
                // play sound when the unit is completed
                if (targetUnit.completeSound)
                {
                    scene->playSoundOnSelectChannel(*targetUnit.completeSound);
                }
                if (targetUnit.activateWhenBuilt)
                {
                    scene->activateUnit(buildingState->targetUnit);
                }
            }
        }

        return false;
    }

    bool UnitBehaviorService::handleBuggerOffOrder(UnitId unitId, const BuggerOffOrder& buggerOffOrder)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);
        if (auto idleState = std::get_if<IdleState>(&unit.behaviourState); idleState != nullptr)
        {
            // request a path to get to the build site
            auto destRect = buggerOffOrder.rect.expand((unit.footprintX * 3) - 4, (unit.footprintZ * 3) - 4);
            scene->getSimulation().requestPath(unitId);
            unit.behaviourState = MovingState{destRect, std::nullopt, true};
        }
        else if (auto movingState = std::get_if<MovingState>(&unit.behaviourState); movingState != nullptr)
        {
            // if we are colliding, request a new path
            if (unit.inCollision && !movingState->pathRequested)
            {
                auto& sim = scene->getSimulation();

                // only request a new path if we don't have one yet,
                // or we've already had our current one for a bit
                if (!movingState->path || (sim.gameTime - movingState->path->pathCreationTime) >= GameTime(60))
                {
                    sim.requestPath(unitId);
                    movingState->pathRequested = true;
                }
            }

            // if a path is available, attempt to follow it
            auto& pathToFollow = movingState->path;
            if (pathToFollow)
            {
                if (followPath(unit, *pathToFollow))
                {
                    unit.behaviourState = IdleState();
                    return true;
                }
            }
        }

        return false;
    }

    bool UnitBehaviorService::handleBuild(UnitId unitId, const std::string& unitType)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        return match(
            unit.factoryState,
            [&](const FactoryStateIdle&) {
                scene->activateUnit(unitId);
                unit.factoryState = FactoryStateBuilding();
                return false;
            },
            [&](FactoryStateBuilding& state) {
                if (!unit.inBuildStance)
                {
                    return false;
                }

                auto& sim = scene->getSimulation();

                auto buildPieceInfo = getBuildPieceInfo(unitId);
                buildPieceInfo.position.y = sim.terrain.getHeightAt(buildPieceInfo.position.x, buildPieceInfo.position.z);
                if (!state.targetUnit)
                {
                    state.targetUnit = scene->spawnUnit(unitType, unit.owner, buildPieceInfo.position);
                    if (state.targetUnit)
                    {
                        unit.cobEnvironment->createThread("StartBuilding");
                    }

                    return false;
                }

                auto& targetUnit = scene->getSimulation().getUnit(*state.targetUnit);
                if (targetUnit.unitType != unitType)
                {
                    if (targetUnit.isBeingBuilt() && !targetUnit.isDead())
                    {
                        scene->quietlyKillUnit(*state.targetUnit);
                    }
                    state.targetUnit = std::nullopt;
                    return false;
                }

                if (targetUnit.isDead())
                {
                    unit.cobEnvironment->createThread("StopBuilding");
                    scene->deactivateUnit(unitId);
                    unit.factoryState = FactoryStateIdle();
                    return true;
                }

                if (!targetUnit.isBeingBuilt())
                {
                    if (targetUnit.orders.empty())
                    {
                        auto footprintRect = scene->computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
                        targetUnit.addOrder(BuggerOffOrder(footprintRect));
                    }
                    unit.cobEnvironment->createThread("StopBuilding");
                    scene->deactivateUnit(unitId);
                    unit.factoryState = FactoryStateIdle();
                    return true;
                }

                tryApplyMovementToPosition(*state.targetUnit, buildPieceInfo.position);
                targetUnit.rotation = buildPieceInfo.rotation;

                auto costs = targetUnit.getBuildCostInfo(unit.workerTimePerTick);
                auto gotResources = sim.addResourceDelta(
                    unitId,
                    -Energy(targetUnit.energyCost.value * static_cast<float>(unit.workerTimePerTick) / static_cast<float>(targetUnit.buildTime)),
                    -Metal(targetUnit.metalCost.value * static_cast<float>(unit.workerTimePerTick) / static_cast<float>(targetUnit.buildTime)),
                    -costs.energyCost,
                    -costs.metalCost);

                if (!gotResources)
                {
                    // we don't have resources available to build -- wait
                    return false;
                }

                if (targetUnit.addBuildProgress(unit.workerTimePerTick))
                {
                    // play sound when the unit is completed
                    if (targetUnit.completeSound)
                    {
                        scene->playSoundOnSelectChannel(*targetUnit.completeSound);
                    }
                    if (targetUnit.activateWhenBuilt)
                    {
                        scene->activateUnit(*state.targetUnit);
                    }
                }

                return false;
            });
    }

    void UnitBehaviorService::clearBuild(UnitId unitId)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        match(
            unit.factoryState,
            [&](const FactoryStateIdle&) {
                // do nothing
            },
            [&](const FactoryStateBuilding& state) {
                if (state.targetUnit)
                {
                    scene->quietlyKillUnit(*state.targetUnit);
                    unit.cobEnvironment->createThread("StopBuilding");
                }
                scene->deactivateUnit(unitId);
                unit.factoryState = FactoryStateIdle();
            });
    }

    SimVector UnitBehaviorService::getNanoPoint(UnitId id)
    {
        auto pieceId = runCobQuery(id, "QueryNanoPiece");
        if (!pieceId)
        {
            return scene->getSimulation().getUnit(id).position;
        }

        return getPiecePosition(id, *pieceId);
    }

    SimVector UnitBehaviorService::getPiecePosition(UnitId id, unsigned int pieceId)
    {
        auto& unit = scene->getSimulation().getUnit(id);

        const auto& pieceName = unit.cobEnvironment->_script->pieces.at(pieceId);
        auto pieceTransform = unit.mesh.getPieceTransform(pieceName);
        if (!pieceTransform)
        {
            throw std::logic_error("Failed to find piece offset");
        }

        return unit.getTransform() * (*pieceTransform) * SimVector(0_ss, 0_ss, 0_ss);
    }

    SimAngle angleTo(const Vector2x<SimScalar>& lhs, const Vector2x<SimScalar>& rhs)
    {
        return atan2(lhs.det(rhs), lhs.dot(rhs));
    }

    SimAngle UnitBehaviorService::getPieceXZRotation(UnitId id, unsigned int pieceId)
    {
        auto& unit = scene->getSimulation().getUnit(id);

        const auto& pieceName = unit.cobEnvironment->_script->pieces.at(pieceId);
        auto pieceTransform = unit.mesh.getPieceTransform(pieceName);
        if (!pieceTransform)
        {
            throw std::logic_error("Failed to find piece offset");
        }

        auto mat = unit.getTransform() * (*pieceTransform);

        auto a = Vector2x<SimScalar>(0_ss, 1_ss);
        auto b = mat.mult3x3(SimVector(0_ss, 0_ss, 1_ss)).xz();
        if (b.lengthSquared() == 0_ss)
        {
            return SimAngle(0);
        }

        // angleTo is computed in a space where Y points up,
        // but in our XZ space (Z is our Y here), Z points down.
        // This means we need to negate (and rewrap) the rotation value.
        return -angleTo(a, b);
    }

    UnitBehaviorService::BuildPieceInfo UnitBehaviorService::getBuildPieceInfo(UnitId id)
    {
        auto pieceId = runCobQuery(id, "QueryBuildInfo");
        if (!pieceId)
        {
            const auto& unit = scene->getSimulation().getUnit(id);
            return BuildPieceInfo{unit.position, unit.rotation};
        }

        return BuildPieceInfo{getPiecePosition(id, *pieceId), getPieceXZRotation(id, *pieceId)};
    }

    std::optional<SimVector> UnitBehaviorService::getTargetPosition(const UnitWeaponAttackTarget& target)
    {
        return match(
            target,
            [](const SimVector& v) { return std::make_optional(v); },
            [this](UnitId id) { return tryGetSweetSpot(id); });
    }

    MovingStateGoal UnitBehaviorService::attackTargetToMovingStateGoal(const AttackTarget& target)
    {
        return match(
            target,
            [](const SimVector& target) { return MovingStateGoal(target); },
            [this](UnitId unitId) {
                const auto& targetUnit = scene->getSimulation().getUnit(unitId);
                return MovingStateGoal(scene->computeFootprintRegion(targetUnit.position, targetUnit.footprintX, targetUnit.footprintZ));
            });
    }
}
