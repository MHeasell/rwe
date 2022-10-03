#include "UnitBehaviorService.h"
#include <rwe/Index.h>
#include <rwe/cob/CobExecutionContext.h>
#include <rwe/geometry/Circle2x.h>
#include <rwe/match.h>
#include <rwe/math/rwe_math.h>
#include <rwe/sim/cob.h>
#include <rwe/sim/movement.h>

namespace rwe
{
    SimScalar getTurnRadius(SimScalar speed, SimScalar turnRate)
    {
        return speed / angularToRadians(turnRate);
    }

    UnitBehaviorService::UnitBehaviorService(GameSimulation* sim)
        : sim(sim)
    {
    }

    void UnitBehaviorService::onCreate(UnitId unitId)
    {
        auto& unit = sim->getUnitState(unitId);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);

        unit.cobEnvironment->createThread("Create", std::vector<int>());

        // set speed for metal extractors
        if (unitDefinition.extractsMetal != Metal(0))
        {
            auto footprint = sim->computeFootprintRegion(unit.position, unitDefinition.movementCollisionInfo);
            auto metalValue = sim->metalGrid.accumulate(sim->metalGrid.clipRegion(footprint), 0u, std::plus<>());
            unit.cobEnvironment->createThread("SetSpeed", {static_cast<int>(metalValue)});
        }

        runUnitCobScripts(*sim, unitId);

        // measure z distances for ballistics
        for (int i = 0; i < getSize(unit.weapons); ++i)
        {
            auto& weapon = unit.weapons[i];
            if (!weapon)
            {
                continue;
            }
            auto localAimingPoint = getLocalAimingPoint(unitId, i);
            auto localFiringPoint = getLocalFiringPoint(unitId, i);
            weapon->ballisticZOffset = localFiringPoint.z - localAimingPoint.z;
        }
    }

    void UnitBehaviorService::update(UnitId unitId)
    {
        auto& unit = sim->getUnitState(unitId);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);

        // Clear steering targets.
        unit.steeringInfo = SteeringInfo{
            unit.rotation,
            0_ss,
        };

        // Run unit and weapon AI
        if (!unit.isBeingBuilt(unitDefinition))
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
                    unit.buildOrderUnitId = std::nullopt;
                }
            }
            else
            {
                changeState(unit, UnitBehaviorStateIdle());
            }

            for (Index i = 0; i < getSize(unit.weapons); ++i)
            {
                updateWeapon(unitId, i);
            }
        }

        if (unitDefinition.isMobile)
        {
            applyUnitSteering(unitId);

            auto previouslyWasMoving = !areCloserThan(unit.previousPosition, unit.position, 0.1_ssf);

            updateUnitPosition(unitId);

            auto currentlyIsMoving = !areCloserThan(unit.previousPosition, unit.position, 0.1_ssf);

            if (currentlyIsMoving && !previouslyWasMoving)
            {
                unit.cobEnvironment->createThread("StartMoving");
            }
            else if (!currentlyIsMoving && previouslyWasMoving)
            {
                unit.cobEnvironment->createThread("StopMoving");
            }
        }
    }

    std::pair<SimAngle, SimAngle> UnitBehaviorService::computeHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to, SimScalar speed, SimScalar gravity, SimScalar zOffset, ProjectilePhysicsType projectileType)
    {
        switch (projectileType)
        {
            case ProjectilePhysicsType::LineOfSight:
                return computeLineOfSightHeadingAndPitch(rotation, from, to);
            case ProjectilePhysicsType::Ballistic:
                return computeBallisticHeadingAndPitch(rotation, from, to, speed, gravity, zOffset);
            default:
                throw std::logic_error("Unknown ProjectilePhysicsType");
        }
    }

    std::pair<SimAngle, SimAngle> UnitBehaviorService::computeLineOfSightHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to)
    {
        auto aimVector = to - from;
        if (aimVector.lengthSquared() == 0_ss)
        {
            aimVector = UnitState::toDirection(rotation);
        }

        SimVector aimVectorXZ(aimVector.x, 0_ss, aimVector.z);

        auto heading = UnitState::toRotation(aimVectorXZ);
        heading = heading - rotation;

        auto pitch = atan2(aimVector.y, aimVectorXZ.length());

        return {heading, pitch};
    }

    std::optional<std::pair<SimAngle, SimAngle>> computeFiringAngles(SimScalar speed, SimScalar gravity, SimScalar targetX, SimScalar targetY)
    {
        auto inner = (gravity * targetX * targetX) + (2_ss * speed * speed * targetY);
        auto beforeSquareRoot = (speed * speed * speed * speed) - (gravity * inner);
        if (beforeSquareRoot < 0_ss)
        {
            return std::nullopt;
        }
        auto plusMinus = rweSqrt(beforeSquareRoot);

        auto result1 = atan(((speed * speed) + plusMinus) / (gravity * targetX));
        auto result2 = atan(((speed * speed) - plusMinus) / (gravity * targetX));

        return std::make_pair(result1, result2);
    }

    std::pair<SimAngle, SimAngle> UnitBehaviorService::computeBallisticHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to, SimScalar speed, SimScalar gravity, SimScalar zOffset)
    {
        auto aimVector = to - from;
        if (aimVector.lengthSquared() == 0_ss)
        {
            aimVector = UnitState::toDirection(rotation);
        }

        SimVector aimVectorXZ(aimVector.x, 0_ss, aimVector.z);

        auto heading = UnitState::toRotation(aimVectorXZ);
        heading = heading - rotation;

        auto pitches = computeFiringAngles(speed, gravity, aimVectorXZ.length() - zOffset, aimVector.y);
        if (!pitches)
        {
            return {heading, EighthTurn};
        }

        return {heading, pitches->second};
    }

    SteeringInfo seek(const UnitState& unit, const UnitDefinition& unitDefinition, const SimVector& destination)
    {
        SimVector xzPosition(unit.position.x, 0_ss, unit.position.z);
        SimVector xzDestination(destination.x, 0_ss, destination.z);
        auto xzDirection = xzDestination - xzPosition;

        // scale desired speed proportionally to how aligned we are
        // with the target direction
        auto normalizedUnitDirection = UnitState::toDirection(unit.rotation);
        auto normalizedXzDirection = xzDirection.normalized();
        auto speedFactor = rweMax(0_ss, normalizedUnitDirection.dot(normalizedXzDirection));

        // Bias the speed factor towards zero if we are within our turn radius of the goal.
        // This is to try and discourage units from orbiting their destination.
        auto turnRadius = getTurnRadius(unitDefinition.maxVelocity, unitDefinition.turnRate);
        if (xzDirection.lengthSquared() <= turnRadius * turnRadius)
        {
            speedFactor = speedFactor * speedFactor;
        }

        return SteeringInfo{
            UnitState::toRotation(xzDirection),
            unitDefinition.maxVelocity * speedFactor,
        };
    }

    SteeringInfo arrive(const UnitState& unit, const UnitDefinition& unitDefinition, const SimVector& destination)
    {
        SimVector xzPosition(unit.position.x, 0_ss, unit.position.z);
        SimVector xzDestination(destination.x, 0_ss, destination.z);
        auto distanceSquared = xzPosition.distanceSquared(xzDestination);
        auto brakingDistance = (unit.currentSpeed * unit.currentSpeed) / (2_ss * unitDefinition.brakeRate);

        if (distanceSquared > (brakingDistance * brakingDistance))
        {
            return seek(unit, unitDefinition, destination);
        }

        // slow down when approaching the destination
        auto xzDirection = xzDestination - xzPosition;
        return SteeringInfo{
            UnitState::toRotation(xzDirection),
            0_ss,
        };
    }

    bool followPath(UnitState& unit, const UnitDefinition& unitDefinition, PathFollowingInfo& path)
    {
        const auto& destination = *path.currentWaypoint;
        SimVector xzPosition(unit.position.x, 0_ss, unit.position.z);
        SimVector xzDestination(destination.x, 0_ss, destination.z);
        auto distanceSquared = xzPosition.distanceSquared(xzDestination);

        auto isFinalDestination = path.currentWaypoint == (path.path.waypoints.end() - 1);

        if (isFinalDestination)
        {
            if (distanceSquared < (8_ss * 8_ss))
            {
                return true;
            }

            unit.steeringInfo = arrive(unit, unitDefinition, destination);
            return false;
        }

        if (distanceSquared < (16_ss * 16_ss))
        {
            ++path.currentWaypoint;
            return false;
        }

        unit.steeringInfo = seek(unit, unitDefinition, destination);
        return false;
    }

    void UnitBehaviorService::updateWeapon(UnitId id, unsigned int weaponIndex)
    {
        auto& unit = sim->getUnitState(id);
        auto& weapon = unit.weapons[weaponIndex];
        if (!weapon)
        {
            return;
        }

        const auto& weaponDefinition = sim->weaponDefinitions.at(weapon->weaponType);

        if (auto idleState = std::get_if<UnitWeaponStateIdle>(&weapon->state); idleState != nullptr)
        {
            // attempt to acquire a target
            if (!weaponDefinition.commandFire && unit.fireOrders == UnitFireOrders::FireAtWill)
            {
                for (const auto& entry : sim->units)
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

                    if (unit.position.distanceSquared(otherUnit.position) > weaponDefinition.maxRange * weaponDefinition.maxRange)
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
            if (std::holds_alternative<UnitWeaponStateAttacking::FireInfo>(aimingState->attackInfo))
            {
                tryFireWeapon(id, weaponIndex);
                return;
            }

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

            if (!targetPosition || unit.position.distanceSquared(*targetPosition) > weaponDefinition.maxRange * weaponDefinition.maxRange)
            {
                unit.clearWeaponTarget(weaponIndex);
            }
            else if (std::holds_alternative<UnitWeaponStateAttacking::IdleInfo>(aimingState->attackInfo))
            {
                auto aimFromPosition = getAimingPoint(id, weaponIndex);

                auto headingAndPitch = computeHeadingAndPitch(unit.rotation, aimFromPosition, *targetPosition, weaponDefinition.velocity, (112_ss / (30_ss * 30_ss)), weapon->ballisticZOffset, weaponDefinition.physicsType);
                auto heading = headingAndPitch.first;
                auto pitch = headingAndPitch.second;

                auto threadId = unit.cobEnvironment->createThread(getAimScriptName(weaponIndex), {toCobAngle(heading).value, toCobAngle(pitch).value});

                if (threadId)
                {
                    aimingState->attackInfo = UnitWeaponStateAttacking::AimInfo{*threadId, heading, pitch};
                }
                else
                {
                    // We couldn't launch an aiming script (there isn't one),
                    // just go straight to firing.
                    aimingState->attackInfo = UnitWeaponStateAttacking::FireInfo{heading, pitch, *targetPosition, std::nullopt, 0, GameTime(0)};
                    tryFireWeapon(id, weaponIndex);
                }
            }
            else if (auto aimInfo = std::get_if<UnitWeaponStateAttacking::AimInfo>(&aimingState->attackInfo))
            {
                auto returnValue = unit.cobEnvironment->tryReapThread(aimInfo->thread);
                if (returnValue)
                {
                    // we successfully reaped, clear the thread.
                    aimingState->attackInfo = UnitWeaponStateAttacking::IdleInfo{};

                    if (*returnValue)
                    {
                        // aiming was successful, check the target again for drift
                        auto aimFromPosition = getAimingPoint(id, weaponIndex);

                        auto headingAndPitch = computeHeadingAndPitch(unit.rotation, aimFromPosition, *targetPosition, weaponDefinition.velocity, (112_ss / (30_ss * 30_ss)), weapon->ballisticZOffset, weaponDefinition.physicsType);
                        auto heading = headingAndPitch.first;
                        auto pitch = headingAndPitch.second;

                        // if the target is close enough, try to fire
                        if (angleBetweenIsLessOrEqual(heading, aimInfo->lastHeading, weaponDefinition.tolerance) && angleBetweenIsLessOrEqual(pitch, aimInfo->lastPitch, weaponDefinition.pitchTolerance))
                        {
                            aimingState->attackInfo = UnitWeaponStateAttacking::FireInfo{heading, pitch, *targetPosition, std::nullopt, 0, GameTime(0)};
                            tryFireWeapon(id, weaponIndex);
                        }
                    }
                }
            }
        }
    }

    SimVector toDirection(SimAngle heading, SimAngle pitch)
    {
        return Matrix4x<SimScalar>::rotationY(sin(heading), cos(heading))
            * Matrix4x<SimScalar>::rotationX(sin(pitch), cos(pitch))
            * SimVector(0_ss, 0_ss, 1_ss);
    }

    SimVector rotateDirectionXZ(const SimVector& direction, SimAngle angle)
    {
        return Matrix4x<SimScalar>::rotationY(sin(angle), cos(angle)) * direction;
    }

    SimVector UnitBehaviorService::changeDirectionByRandomAngle(const SimVector& direction, SimAngle maxAngle)
    {
        std::uniform_int_distribution dist(SimAngle(0).value, maxAngle.value);
        std::uniform_int_distribution dist2(0, 1);
        auto& rng = sim->rng;
        auto angle = SimAngle(dist(rng));
        if (dist2(rng))
        {
            angle = SimAngle(0) - angle;
        }

        return rotateDirectionXZ(direction, angle);
    }

    void UnitBehaviorService::tryFireWeapon(UnitId id, unsigned int weaponIndex)
    {
        auto& unit = sim->getUnitState(id);
        auto& weapon = unit.weapons[weaponIndex];

        if (!weapon)
        {
            return;
        }

        const auto& weaponDefinition = sim->weaponDefinitions.at(weapon->weaponType);

        auto attackInfo = std::get_if<UnitWeaponStateAttacking>(&weapon->state);
        if (!attackInfo)
        {
            return;
        }

        auto fireInfo = std::get_if<UnitWeaponStateAttacking::FireInfo>(&attackInfo->attackInfo);
        if (!fireInfo)
        {
            return;
        }

        // wait for the weapon to reload before firing first burst
        auto gameTime = sim->gameTime;
        if (fireInfo->burstsFired == 0 && gameTime < weapon->readyTime)
        {
            return;
        }

        // wait for burst reload
        if (gameTime < fireInfo->readyTime)
        {
            return;
        }

        // spawn a projectile from the firing point
        if (!fireInfo->firingPiece)
        {
            auto scriptName = getQueryScriptName(weaponIndex);
            fireInfo->firingPiece = runCobQuery(id, scriptName).value_or(0);
        }

        auto firingPoint = unit.getTransform() * getPieceLocalPosition(id, *fireInfo->firingPiece);

        SimVector direction;
        switch (weaponDefinition.physicsType)
        {
            case ProjectilePhysicsType::LineOfSight:
                direction = (fireInfo->targetPosition - firingPoint).normalized();
                break;
            case ProjectilePhysicsType::Ballistic:
                direction = toDirection(fireInfo->heading + unit.rotation, -fireInfo->pitch);
                break;
            default:
                throw std::logic_error("Unknown ProjectilePhysicsType");
        }

        if (weaponDefinition.sprayAngle != SimAngle(0))
        {
            direction = changeDirectionByRandomAngle(direction, weaponDefinition.sprayAngle);
        }

        sim->spawnProjectile(unit.owner, *weapon, firingPoint, direction, (fireInfo->targetPosition - firingPoint).length());

        sim->events.push_back(FireWeaponEvent{weapon->weaponType, fireInfo->burstsFired, firingPoint});

        // If we just started the burst, set the reload timer
        if (fireInfo->burstsFired == 0)
        {
            unit.cobEnvironment->createThread(getFireScriptName(weaponIndex));
            weapon->readyTime = gameTime + deltaSecondsToTicks(weaponDefinition.reloadTime);
        }

        ++fireInfo->burstsFired;
        fireInfo->readyTime = gameTime + deltaSecondsToTicks(weaponDefinition.burstInterval);
        if (fireInfo->burstsFired >= weaponDefinition.burst)
        {
            // we finished our burst, we are reloading now
            attackInfo->attackInfo = UnitWeaponStateAttacking::IdleInfo{};
        }
    }

    void UnitBehaviorService::applyUnitSteering(UnitId id)
    {
        updateUnitRotation(id);
        updateUnitSpeed(id);
    }

    void UnitBehaviorService::updateUnitRotation(UnitId id)
    {
        auto& unit = sim->getUnitState(id);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);
        auto turnRateThisFrame = SimAngle(unitDefinition.turnRate.value);
        unit.previousRotation = unit.rotation;
        unit.rotation = turnTowards(unit.rotation, unit.steeringInfo.targetAngle, turnRateThisFrame);
    }

    void UnitBehaviorService::updateUnitSpeed(UnitId id)
    {
        auto& unit = sim->getUnitState(id);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);

        const auto& steeringInfo = unit.steeringInfo;

        if (steeringInfo.targetSpeed > unit.currentSpeed)
        {
            // accelerate to target speed
            if (steeringInfo.targetSpeed - unit.currentSpeed <= unitDefinition.acceleration)
            {
                unit.currentSpeed = steeringInfo.targetSpeed;
            }
            else
            {
                unit.currentSpeed += unitDefinition.acceleration;
            }
        }
        else
        {
            // brake to target speed
            if (unit.currentSpeed - steeringInfo.targetSpeed <= unitDefinition.brakeRate)
            {
                unit.currentSpeed = steeringInfo.targetSpeed;
            }
            else
            {
                unit.currentSpeed -= unitDefinition.brakeRate;
            }
        }

        auto effectiveMaxSpeed = unitDefinition.maxVelocity;
        if (unit.position.y < sim->terrain.getSeaLevel())
        {
            effectiveMaxSpeed /= 2_ss;
        }
        unit.currentSpeed = std::clamp(unit.currentSpeed, 0_ss, effectiveMaxSpeed);
    }

    void UnitBehaviorService::updateUnitPosition(UnitId unitId)
    {
        auto& unit = sim->getUnitState(unitId);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);

        unit.previousPosition = unit.position;

        auto direction = UnitState::toDirection(unit.rotation);

        unit.inCollision = false;

        if (unit.currentSpeed > 0_ss)
        {
            auto newPosition = unit.position + (direction * unit.currentSpeed);
            newPosition.y = sim->terrain.getHeightAt(newPosition.x, newPosition.z);
            if (unitDefinition.floater || unitDefinition.canHover)
            {
                newPosition.y = rweMax(newPosition.y, sim->terrain.getSeaLevel());
            }

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
                newPos1.y = sim->terrain.getHeightAt(newPos1.x, newPos1.z);
                newPos2.y = sim->terrain.getHeightAt(newPos2.x, newPos2.z);

                if (unitDefinition.floater || unitDefinition.canHover)
                {
                    newPos1.y = rweMax(newPos1.y, sim->terrain.getSeaLevel());
                    newPos2.y = rweMax(newPos2.y, sim->terrain.getSeaLevel());
                }

                if (!tryApplyMovementToPosition(unitId, newPos1))
                {
                    tryApplyMovementToPosition(unitId, newPos2);
                }
            }
        }
    }

    bool UnitBehaviorService::tryApplyMovementToPosition(UnitId id, const SimVector& newPosition)
    {
        auto& unit = sim->getUnitState(id);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);

        // check for collision at the new position
        auto newFootprintRegion = sim->computeFootprintRegion(newPosition, unitDefinition.movementCollisionInfo);

        if (sim->isCollisionAt(newFootprintRegion, id))
        {
            return false;
        }

        // Unlike for pathfinding, TA doesn't care about the unit's actual movement class for collision checks,
        // it only cares about the attributes defined directly on the unit.
        // Jam these into an ad-hoc movement class to pass into our walkability check.
        if (!isGridPointWalkable(sim->terrain, sim->getAdHocMovementClass(unitDefinition.movementCollisionInfo), newFootprintRegion.x, newFootprintRegion.y))
        {
            return false;
        }

        // we passed all collision checks, update accordingly
        auto footprintRegion = sim->computeFootprintRegion(unit.position, unitDefinition.movementCollisionInfo);
        sim->moveUnitOccupiedArea(footprintRegion, newFootprintRegion, id);

        auto seaLevel = sim->terrain.getSeaLevel();
        auto oldTerrainHeight = sim->terrain.getHeightAt(unit.position.x, unit.position.z);
        auto oldPosBelowSea = oldTerrainHeight < seaLevel;

        unit.position = newPosition;

        auto newTerrainHeight = sim->terrain.getHeightAt(unit.position.x, unit.position.z);
        auto newPosBelowSea = newTerrainHeight < seaLevel;

        if (oldPosBelowSea && !newPosBelowSea)
        {
            unit.cobEnvironment->createThread("setSFXoccupy", std::vector<int>{4});
        }
        else if (!oldPosBelowSea && newPosBelowSea)
        {
            unit.cobEnvironment->createThread("setSFXoccupy", std::vector<int>{2});
        }

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
                throw std::logic_error("Invalid weapon index: " + std::to_string(weaponIndex));
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
        auto& unit = sim->getUnitState(id);
        auto thread = unit.cobEnvironment->createNonScheduledThread(name, {0});
        if (!thread)
        {
            return std::nullopt;
        }
        CobExecutionContext context(unit.cobEnvironment.get(), &*thread, id);
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
        const auto& unit = sim->getUnitState(id);
        return unit.getTransform() * getLocalAimingPoint(id, weaponIndex);
    }

    SimVector UnitBehaviorService::getLocalAimingPoint(UnitId id, unsigned int weaponIndex)
    {
        auto scriptName = getAimFromScriptName(weaponIndex);
        auto pieceId = runCobQuery(id, scriptName);
        if (!pieceId)
        {
            return getLocalFiringPoint(id, weaponIndex);
        }

        return getPieceLocalPosition(id, *pieceId);
    }

    SimVector UnitBehaviorService::getFiringPoint(UnitId id, unsigned int weaponIndex)
    {
        const auto& unit = sim->getUnitState(id);
        return unit.getTransform() * getLocalFiringPoint(id, weaponIndex);
    }

    SimVector UnitBehaviorService::getLocalFiringPoint(UnitId id, unsigned int weaponIndex)
    {

        auto scriptName = getQueryScriptName(weaponIndex);
        auto pieceId = runCobQuery(id, scriptName);
        if (!pieceId)
        {
            return SimVector(0_ss, 0_ss, 0_ss);
        }

        return getPieceLocalPosition(id, *pieceId);
    }

    SimVector UnitBehaviorService::getSweetSpot(UnitId id)
    {
        auto pieceId = runCobQuery(id, "SweetSpot");
        if (!pieceId)
        {
            return sim->getUnitState(id).position;
        }

        return getPiecePosition(id, *pieceId);
    }

    std::optional<SimVector> UnitBehaviorService::tryGetSweetSpot(UnitId id)
    {
        if (!sim->unitExists(id))
        {
            return std::nullopt;
        }

        return getSweetSpot(id);
    }

    bool UnitBehaviorService::handleOrder(UnitId unitId, const UnitOrder& order)
    {
        return match(
            order,
            [this, unitId](const MoveOrder& o)
            {
                return handleMoveOrder(unitId, o);
            },
            [this, unitId](const AttackOrder& o)
            {
                return handleAttackOrder(unitId, o);
            },
            [this, unitId](const BuildOrder& o)
            {
                return handleBuildOrder(unitId, o);
            },
            [this, unitId](const BuggerOffOrder& o)
            {
                return handleBuggerOffOrder(unitId, o);
            },
            [this, unitId](const CompleteBuildOrder& o)
            {
                return handleCompleteBuildOrder(unitId, o);
            },
            [this, unitId](const GuardOrder& o)
            {
                return handleGuardOrder(unitId, o);
            });
    }

    bool UnitBehaviorService::handleMoveOrder(UnitId unitId, const MoveOrder& moveOrder)
    {
        auto& unit = sim->getUnitState(unitId);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);
        if (!unitDefinition.isMobile)
        {
            return false;
        }

        if (moveTo(unitId, moveOrder.destination))
        {
            sim->events.push_back(UnitArrivedEvent{unitId});
            return true;
        }

        return false;
    }

    bool UnitBehaviorService::handleAttackOrder(UnitId unitId, const AttackOrder& attackOrder)
    {
        return attackTarget(unitId, attackOrder.target);
    }

    bool UnitBehaviorService::attackTarget(UnitId unitId, const AttackTarget& target)
    {
        auto& unit = sim->getUnitState(unitId);

        if (!unit.weapons[0])
        {
            return true;
        }

        const auto& weaponDefinition = sim->weaponDefinitions.at(unit.weapons[0]->weaponType);

        auto targetPosition = getTargetPosition(target);
        if (!targetPosition)
        {
            // target has gone away, throw away this order
            return true;
        }

        auto maxRangeSquared = weaponDefinition.maxRange * weaponDefinition.maxRange;
        if (unit.position.distanceSquared(*targetPosition) > maxRangeSquared)
        {
            moveTo(unitId, *targetPosition);
        }
        else
        {
            // we're in range, aim weapons
            for (unsigned int i = 0; i < 2; ++i)
            {
                match(
                    target,
                    [&](const UnitId& u)
                    { unit.setWeaponTarget(i, u); },
                    [&](const SimVector& v)
                    { unit.setWeaponTarget(i, v); });
            }
        }

        return false;
    }

    bool UnitBehaviorService::handleBuildOrder(UnitId unitId, const BuildOrder& buildOrder)
    {
        return buildUnit(unitId, buildOrder.unitType, buildOrder.position);
    }

    bool UnitBehaviorService::handleBuggerOffOrder(UnitId unitId, const BuggerOffOrder& buggerOffOrder)
    {
        auto& unit = sim->getUnitState(unitId);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);
        auto [footprintX, footprintZ] = sim->getFootprintXZ(unitDefinition.movementCollisionInfo);
        return moveTo(unitId, buggerOffOrder.rect.expand((footprintX * 3) - 4, (footprintZ * 3) - 4));
    }

    bool UnitBehaviorService::handleCompleteBuildOrder(rwe::UnitId unitId, const rwe::CompleteBuildOrder& buildOrder)
    {
        return buildExistingUnit(unitId, buildOrder.target);
    }

    bool UnitBehaviorService::handleGuardOrder(UnitId unitId, const GuardOrder& guardOrder)
    {
        auto& unit = sim->getUnitState(unitId);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);

        auto target = sim->tryGetUnitState(guardOrder.target);
        // TODO: real allied check here
        if (!target || !target->get().isOwnedBy(unit.owner))
        {
            // unit is dead or a traitor, abandon order
            return true;
        }
        auto& targetUnit = target->get();


        // assist building
        if (auto bs = std::get_if<UnitBehaviorStateBuilding>(&targetUnit.behaviourState); unitDefinition.builder && bs)
        {
            buildExistingUnit(unitId, bs->targetUnit);
            return false;
        }

        // assist factory building
        if (auto fs = std::get_if<FactoryBehaviorStateBuilding>(&targetUnit.factoryState); unitDefinition.builder && fs)
        {
            if (fs->targetUnit)
            {
                buildExistingUnit(unitId, fs->targetUnit->first);
                return false;
            }
        }

        // stay close
        if (unitDefinition.canMove && unit.position.distanceSquared(targetUnit.position) > SimScalar(200 * 200))
        {
            moveTo(unitId, targetUnit.position);
            return false;
        }

        return false;
    }

    bool UnitBehaviorService::handleBuild(UnitId unitId, const std::string& unitType)
    {
        auto& unit = sim->getUnitState(unitId);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);


        return match(
            unit.factoryState,
            [&](const FactoryBehaviorStateIdle&)
            {
                sim->activateUnit(unitId);
                unit.factoryState = FactoryBehaviorStateBuilding();
                return false;
            },
            [&](FactoryBehaviorStateCreatingUnit& state)
            {
                return match(
                    state.status,
                    [&](const UnitCreationStatusPending&)
                    {
                        return false;
                    },
                    [&](const UnitCreationStatusDone& s)
                    {
                        unit.cobEnvironment->createThread("StartBuilding");
                        unit.factoryState = FactoryBehaviorStateBuilding{std::make_pair(s.unitId, std::optional<SimVector>())};
                        return false;
                    },
                    [&](const UnitCreationStatusFailed&)
                    {
                        unit.factoryState = FactoryBehaviorStateBuilding();
                        return false;
                    });
            },
            [&](FactoryBehaviorStateBuilding& state)
            {
                if (!unit.inBuildStance)
                {
                    return false;
                }

                auto buildPieceInfo = getBuildPieceInfo(unitId);
                // buildPieceInfo.position.y = sim->terrain.getHeightAt(buildPieceInfo.position.x, buildPieceInfo.position.z);
                if (!state.targetUnit)
                {
                    unit.factoryState = FactoryBehaviorStateCreatingUnit{unitType, unit.owner, buildPieceInfo.position, buildPieceInfo.rotation};
                    sim->unitCreationRequests.push_back(unitId);
                    return false;
                }

                auto targetUnitOption = sim->tryGetUnitState(state.targetUnit->first);
                if (!targetUnitOption)
                {
                    unit.factoryState = FactoryBehaviorStateCreatingUnit{unitType, unit.owner, buildPieceInfo.position, buildPieceInfo.rotation};
                    sim->unitCreationRequests.push_back(unitId);
                    return false;
                }

                auto& targetUnit = targetUnitOption->get();
                const auto& targetUnitDefinition = sim->unitDefinitions.at(targetUnit.unitType);

                if (targetUnit.unitType != unitType)
                {
                    if (targetUnit.isBeingBuilt(targetUnitDefinition) && !targetUnit.isDead())
                    {
                        sim->quietlyKillUnit(state.targetUnit->first);
                    }
                    state.targetUnit = std::nullopt;
                    return false;
                }

                if (targetUnit.isDead())
                {
                    unit.cobEnvironment->createThread("StopBuilding");
                    sim->deactivateUnit(unitId);
                    unit.factoryState = FactoryBehaviorStateIdle();
                    return true;
                }

                if (!targetUnit.isBeingBuilt(targetUnitDefinition))
                {
                    if (unit.orders.empty())
                    {
                        auto footprintRect = sim->computeFootprintRegion(unit.position, unitDefinition.movementCollisionInfo);
                        targetUnit.addOrder(BuggerOffOrder(footprintRect));
                    }
                    else
                    {
                        targetUnit.replaceOrders(unit.orders);
                    }
                    unit.cobEnvironment->createThread("StopBuilding");
                    sim->deactivateUnit(unitId);
                    unit.factoryState = FactoryBehaviorStateIdle();
                    return true;
                }

                if (targetUnitDefinition.floater || targetUnitDefinition.canHover)
                {
                    buildPieceInfo.position.y = rweMax(buildPieceInfo.position.y, sim->terrain.getSeaLevel());
                }

                tryApplyMovementToPosition(state.targetUnit->first, buildPieceInfo.position);
                targetUnit.rotation = buildPieceInfo.rotation;

                auto costs = targetUnit.getBuildCostInfo(targetUnitDefinition, unitDefinition.workerTimePerTick);
                auto gotResources = sim->addResourceDelta(
                    unitId,
                    -Energy(targetUnitDefinition.buildCostEnergy.value * static_cast<float>(unitDefinition.workerTimePerTick) / static_cast<float>(targetUnitDefinition.buildTime)),
                    -Metal(targetUnitDefinition.buildCostMetal.value * static_cast<float>(unitDefinition.workerTimePerTick) / static_cast<float>(targetUnitDefinition.buildTime)),
                    -costs.energyCost,
                    -costs.metalCost);

                if (!gotResources)
                {
                    // we don't have resources available to build -- wait
                    state.targetUnit->second = std::nullopt;
                    return false;
                }
                state.targetUnit->second = getNanoPoint(unitId);

                if (targetUnit.addBuildProgress(targetUnitDefinition, unitDefinition.workerTimePerTick))
                {
                    sim->events.push_back(UnitCompleteEvent{state.targetUnit->first});

                    if (targetUnitDefinition.activateWhenBuilt)
                    {
                        sim->activateUnit(state.targetUnit->first);
                    }
                }

                return false;
            });
    }

    void UnitBehaviorService::clearBuild(UnitId unitId)
    {
        auto& unit = sim->getUnitState(unitId);

        match(
            unit.factoryState,
            [&](const FactoryBehaviorStateIdle&)
            {
                // do nothing
            },
            [&](const FactoryBehaviorStateCreatingUnit& state)
            {
                match(
                    state.status,
                    [&](const UnitCreationStatusDone& d)
                    {
                        sim->quietlyKillUnit(d.unitId);
                    },
                    [&](const auto&)
                    {
                        // do nothing
                    });
                sim->deactivateUnit(unitId);
                unit.factoryState = FactoryBehaviorStateIdle();
            },
            [&](const FactoryBehaviorStateBuilding& state)
            {
                if (state.targetUnit)
                {
                    sim->quietlyKillUnit(state.targetUnit->first);
                    unit.cobEnvironment->createThread("StopBuilding");
                }
                sim->deactivateUnit(unitId);
                unit.factoryState = FactoryBehaviorStateIdle();
            });
    }

    SimVector UnitBehaviorService::getNanoPoint(UnitId id)
    {
        auto pieceId = runCobQuery(id, "QueryNanoPiece");
        if (!pieceId)
        {
            return sim->getUnitState(id).position;
        }

        return getPiecePosition(id, *pieceId);
    }

    SimVector UnitBehaviorService::getPieceLocalPosition(UnitId id, unsigned int pieceId)
    {
        auto& unit = sim->getUnitState(id);

        const auto& pieceName = unit.cobEnvironment->_script->pieces.at(pieceId);
        auto pieceTransform = sim->getUnitPieceLocalTransform(id, pieceName);

        return pieceTransform * SimVector(0_ss, 0_ss, 0_ss);
    }

    SimVector UnitBehaviorService::getPiecePosition(UnitId id, unsigned int pieceId)
    {
        auto& unit = sim->getUnitState(id);

        return unit.getTransform() * getPieceLocalPosition(id, pieceId);
    }

    SimAngle angleTo(const Vector2x<SimScalar>& lhs, const Vector2x<SimScalar>& rhs)
    {
        return atan2(lhs.det(rhs), lhs.dot(rhs));
    }

    SimAngle UnitBehaviorService::getPieceXZRotation(UnitId id, unsigned int pieceId)
    {
        auto& unit = sim->getUnitState(id);

        const auto& pieceName = unit.cobEnvironment->_script->pieces.at(pieceId);
        auto pieceTransform = sim->getUnitPieceLocalTransform(id, pieceName);

        auto mat = unit.getTransform() * pieceTransform;

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
            const auto& unit = sim->getUnitState(id);
            return BuildPieceInfo{unit.position, unit.rotation};
        }

        return BuildPieceInfo{getPiecePosition(id, *pieceId), getPieceXZRotation(id, *pieceId)};
    }

    std::optional<SimVector> UnitBehaviorService::getTargetPosition(const UnitWeaponAttackTarget& target)
    {
        return match(
            target,
            [](const SimVector& v)
            { return std::make_optional(v); },
            [this](UnitId id)
            { return tryGetSweetSpot(id); });
    }

    MovingStateGoal UnitBehaviorService::attackTargetToMovingStateGoal(const AttackTarget& target)
    {
        return match(
            target,
            [&](const SimVector& target)
            { return MovingStateGoal(target); },
            [&](UnitId unitId)
            {
                const auto& targetUnit = sim->getUnitState(unitId);
                const auto& targetUnitDefinition = sim->unitDefinitions.at(targetUnit.unitType);
                return MovingStateGoal(sim->computeFootprintRegion(targetUnit.position, targetUnitDefinition.movementCollisionInfo));
            });
    }

    bool UnitBehaviorService::moveTo(UnitId unitId, const MovingStateGoal& goal)
    {
        auto& unit = sim->getUnitState(unitId);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);

        auto movingState = std::get_if<UnitBehaviorStateMoving>(&unit.behaviourState);

        if (!movingState || movingState->destination != goal)
        {
            // request a path to follow
            changeState(unit, UnitBehaviorStateMoving{goal, std::nullopt, true});
            sim->requestPath(unitId);
            return false;
        }

        // if we are colliding, request a new path
        if (unit.inCollision && !movingState->pathRequested)
        {
            // only request a new path if we don't have one yet,
            // or we've already had our current one for a bit
            if (!movingState->path || (sim->gameTime - movingState->path->pathCreationTime) >= GameTime(30))
            {
                sim->requestPath(unitId);
                movingState->pathRequested = true;
            }
        }

        // if a path is available, attempt to follow it
        if (movingState->path)
        {
            if (followPath(unit, unitDefinition, *movingState->path))
            {
                // we finished following the path,
                // clear our state
                changeState(unit, UnitBehaviorStateIdle());
                return true;
            }
        }

        return false;
    }

    UnitCreationStatus UnitBehaviorService::createNewUnit(UnitId unitId, const std::string& unitType, const SimVector& position)
    {
        auto& unit = sim->getUnitState(unitId);

        if (auto s = std::get_if<UnitBehaviorStateCreatingUnit>(&unit.behaviourState))
        {
            if (s->unitType == unitType && s->position == position)
            {
                return s->status;
            }
        }

        const auto& targetUnitDefinition = sim->unitDefinitions.at(unitType);
        auto footprintRect = sim->computeFootprintRegion(position, targetUnitDefinition.movementCollisionInfo);
        if (moveTo(unitId, footprintRect))
        {
            // TODO: add an additional distance check here -- we may have done  the best
            // we can to move but been prevented by some obstacle, so we are too far away still.
            changeState(unit, UnitBehaviorStateCreatingUnit{unitType, unit.owner, position});
            sim->unitCreationRequests.push_back(unitId);
        }

        return UnitCreationStatusPending();
    }

    bool UnitBehaviorService::buildUnit(UnitId unitId, const std::string& unitType, const SimVector& position)
    {
        auto& unit = sim->getUnitState(unitId);
        if (!unit.buildOrderUnitId)
        {
            auto result = createNewUnit(unitId, unitType, position);
            return match(
                result,
                [&](const UnitCreationStatusPending&)
                { return false; },
                [&](const UnitCreationStatusFailed&)
                { return true; },
                [&](const UnitCreationStatusDone& d)
                {
                    unit.buildOrderUnitId = d.unitId;
                    return deployBuildArm(unitId, d.unitId);
                });
        }

        return deployBuildArm(unitId, *unit.buildOrderUnitId);
    }

    bool UnitBehaviorService::buildExistingUnit(UnitId unitId, UnitId targetUnitId)
    {
        auto& unit = sim->getUnitState(unitId);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);
        auto targetUnitRef = sim->tryGetUnitState(targetUnitId);

        if (!targetUnitRef || targetUnitRef->get().isDead() || !targetUnitRef->get().isBeingBuilt(unitDefinition))
        {
            changeState(unit, UnitBehaviorStateIdle());
            return true;
        }
        auto& targetUnit = targetUnitRef->get();

        // FIXME: this distance measure is wrong
        // Experiment has shown that the distance from which a new building
        // can be started (when caged in) is greater than assist distance,
        // and it appears both measures something more advanced than center <-> center distance.
        if (unit.position.distanceSquared(targetUnit.position) > (unitDefinition.buildDistance * unitDefinition.buildDistance))
        {
            moveTo(unitId, targetUnit.position);
            return false;
        }

        // we're close enough -- actually build the unit
        return deployBuildArm(unitId, targetUnitId);
    }

    void UnitBehaviorService::changeState(UnitState& unit, const UnitBehaviorState& newState)
    {
        if (std::holds_alternative<UnitBehaviorStateBuilding>(unit.behaviourState))
        {
            unit.cobEnvironment->createThread("StopBuilding");
        }
        unit.behaviourState = newState;
    }
    bool UnitBehaviorService::deployBuildArm(UnitId unitId, UnitId targetUnitId)
    {
        auto& unit = sim->getUnitState(unitId);
        const auto& unitDefinition = sim->unitDefinitions.at(unit.unitType);
        auto targetUnitRef = sim->tryGetUnitState(targetUnitId);
        if (!targetUnitRef || targetUnitRef->get().isDead() || !targetUnitRef->get().isBeingBuilt(sim->unitDefinitions.at(targetUnitRef->get().unitType)))
        {
            changeState(unit, UnitBehaviorStateIdle());
            return true;
        }
        auto& targetUnit = targetUnitRef->get();
        const auto& targetUnitDefinition = sim->unitDefinitions.at(targetUnit.unitType);

        return match(
            unit.behaviourState,
            [&](UnitBehaviorStateBuilding& buildingState)
            {
                if (targetUnitId != buildingState.targetUnit)
                {
                    changeState(unit, UnitBehaviorStateIdle());
                    return buildExistingUnit(unitId, targetUnitId);
                }

                if (!unit.inBuildStance)
                {
                    // We are not in the correct stance to build the unit yet, wait.
                    return false;
                }

                auto costs = targetUnit.getBuildCostInfo(targetUnitDefinition, unitDefinition.workerTimePerTick);
                auto gotResources = sim->addResourceDelta(
                    unitId,
                    -Energy(targetUnitDefinition.buildCostEnergy.value * static_cast<float>(unitDefinition.workerTimePerTick) / static_cast<float>(targetUnitDefinition.buildTime)),
                    -Metal(targetUnitDefinition.buildCostMetal.value * static_cast<float>(unitDefinition.workerTimePerTick) / static_cast<float>(targetUnitDefinition.buildTime)),
                    -costs.energyCost,
                    -costs.metalCost);

                if (!gotResources)
                {
                    // we don't have resources available to build -- wait
                    buildingState.nanoParticleOrigin = std::nullopt;
                    return false;
                }
                buildingState.nanoParticleOrigin = getNanoPoint(unitId);

                if (targetUnit.addBuildProgress(targetUnitDefinition, unitDefinition.workerTimePerTick))
                {
                    sim->events.push_back(UnitCompleteEvent{buildingState.targetUnit});

                    if (targetUnitDefinition.activateWhenBuilt)
                    {
                        sim->activateUnit(buildingState.targetUnit);
                    }

                    changeState(unit, UnitBehaviorStateIdle());
                    return true;
                }
                return false;
            },
            [&](const auto&)
            {
                auto nanoFromPosition = getNanoPoint(unitId);
                auto headingAndPitch = computeLineOfSightHeadingAndPitch(unit.rotation, nanoFromPosition, targetUnit.position);
                auto heading = headingAndPitch.first;
                auto pitch = headingAndPitch.second;

                changeState(unit, UnitBehaviorStateBuilding{targetUnitId, std::nullopt});
                unit.cobEnvironment->createThread("StartBuilding", {toCobAngle(heading).value, toCobAngle(pitch).value});
                return false;
            });
    }
}
