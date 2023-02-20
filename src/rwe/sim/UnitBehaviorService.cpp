#include "UnitBehaviorService.h"
#include <rwe/Index.h>
#include <rwe/cob/CobExecutionContext.h>
#include <rwe/match.h>
#include <rwe/math/rwe_math.h>
#include <rwe/sim/UnitBehaviorService_util.h>
#include <rwe/sim/cob.h>
#include <rwe/sim/movement.h>

namespace rwe
{
    SimScalar getTargetAltitude(const MapTerrain& terrain, SimScalar x, SimScalar z, const UnitDefinition& unitDefinition)
    {
        return rweMax(terrain.getHeightAt(x, z), terrain.getSeaLevel()) + unitDefinition.cruiseAltitude;
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
        auto unitInfo = sim->getUnitInfo(unitId);

        // Clear steering targets.
        match(
            unitInfo.state->physics,
            [&](UnitPhysicsInfoGround& p) {
                p.steeringInfo = SteeringInfo{
                    unitInfo.state->rotation,
                    0_ss,
                };
            },
            [&](UnitPhysicsInfoAir& p) {
                match(
                    p.movementState,
                    [&](AirMovementStateFlying& s) {
                        s.targetPosition = unitInfo.state->position;
                    },
                    [&](const AirMovementStateTakingOff&) {
                        // do nothing
                    },
                    [&](const AirMovementStateLanding&) {
                        // do nothing
                    });
            });

        // clear navigation targets
        unitInfo.state->navigationState.desiredDestination = std::nullopt;

        // Run unit and weapon AI
        if (!unitInfo.state->isBeingBuilt(*unitInfo.definition))
        {
            // check our build queue
            if (!unitInfo.state->buildQueue.empty())
            {
                auto& entry = unitInfo.state->buildQueue.front();
                if (handleBuild(unitInfo, entry.first))
                {
                    if (entry.second > 1)
                    {
                        --entry.second;
                    }
                    else
                    {
                        unitInfo.state->buildQueue.pop_front();
                    }
                }
            }
            else
            {
                clearBuild(unitInfo);
            }

            // check our orders
            if (!unitInfo.state->orders.empty())
            {
                const auto& order = unitInfo.state->orders.front();

                // process move orders
                if (handleOrder(unitInfo, order))
                {
                    unitInfo.state->orders.pop_front();
                    unitInfo.state->buildOrderUnitId = std::nullopt;
                }
            }
            else if (auto airPhysics = std::get_if<UnitPhysicsInfoAir>(&unitInfo.state->physics); airPhysics != nullptr)
            {
                match(
                    airPhysics->movementState,
                    [&](AirMovementStateFlying& m) {
                        if (navigateTo(unitInfo, NavigationGoalLandingLocation()))
                        {
                            m.shouldLand = true;
                        }
                    },
                    [&](const AirMovementStateLanding&) {
                        // do nothing
                    },
                    [&](const AirMovementStateTakingOff&) {
                        // do nothing
                    });
            }
            else
            {
                changeState(*unitInfo.state, UnitBehaviorStateIdle());
            }

            for (Index i = 0; i < getSize(unitInfo.state->weapons); ++i)
            {
                updateWeapon(unitId, i);
            }
        }

        if (unitInfo.definition->isMobile)
        {
            updateNavigation(unitInfo);

            applyUnitSteering(unitInfo);

            auto previouslyWasMoving = !areCloserThan(unitInfo.state->previousPosition, unitInfo.state->position, 0.1_ssf);

            updateUnitPosition(unitInfo);

            auto currentlyIsMoving = !areCloserThan(unitInfo.state->previousPosition, unitInfo.state->position, 0.1_ssf);

            if (currentlyIsMoving && !previouslyWasMoving)
            {
                unitInfo.state->cobEnvironment->createThread("StartMoving");
            }
            else if (!currentlyIsMoving && previouslyWasMoving)
            {
                unitInfo.state->cobEnvironment->createThread("StopMoving");
            }

            // do physics transitions
            match(
                unitInfo.state->physics,
                [&](const UnitPhysicsInfoGround& p) {
                    if (p.steeringInfo.shouldTakeOff)
                    {
                        transitionFromGroundToAir(unitInfo);
                    }
                },
                [&](UnitPhysicsInfoAir& p) {
                    match(
                        p.movementState,
                        [&](const AirMovementStateTakingOff&) {
                            auto targetHeight = getTargetAltitude(sim->terrain, unitInfo.state->position.x, unitInfo.state->position.z, *unitInfo.definition);
                            if (unitInfo.state->position.y == targetHeight)
                            {
                                p.movementState = AirMovementStateFlying();
                            }
                        },
                        [&](AirMovementStateLanding& m) {
                            if (m.shouldAbort)
                            {
                                unitInfo.state->activate();
                                p.movementState = AirMovementStateFlying();
                            }
                            else
                            {
                                auto targetHeight = sim->terrain.getHeightAt(unitInfo.state->position.x, unitInfo.state->position.z);
                                if (unitInfo.state->position.y == targetHeight)
                                {
                                    if (!tryTransitionFromAirToGround(unitInfo))
                                    {
                                        m.landingFailed = true;
                                    }
                                }
                            }
                        },
                        [&](const AirMovementStateFlying& m) {
                            if (m.shouldLand)
                            {
                                p.movementState = AirMovementStateLanding();
                                unitInfo.state->deactivate();
                            }
                        });
                });
        }
    }

    void UnitBehaviorService::updateNavigation(UnitInfo unitInfo)
    {
        const auto& goal = unitInfo.state->navigationState.desiredDestination;

        if (!goal)
        {
            unitInfo.state->navigationState.state = NavigationStateIdle();
            return;
        }

        auto resolvedGoal = match(
            *goal,
            [&](const NavigationGoalLandingLocation&) {
                const auto llState = std::get_if<NavigationStateMovingToLandingSpot>(&unitInfo.state->navigationState.state);
                if (llState)
                {
                    return std::make_optional<MovingStateGoal>(llState->landingLocation);
                }
                else
                {
                    auto landingLocation = findLandingLocation(*sim, unitInfo);
                    if (!landingLocation)
                    {
                        return std::optional<MovingStateGoal>();
                    }
                    unitInfo.state->navigationState.state = NavigationStateMovingToLandingSpot{*landingLocation};
                    return std::make_optional<MovingStateGoal>(*landingLocation);
                }
            },
            [&](const SimVector& v) {
                return std::make_optional<MovingStateGoal>(v);
            },
            [&](const DiscreteRect& r) {
                return std::make_optional<MovingStateGoal>(r);
            });

        if (!resolvedGoal)
        {
            unitInfo.state->navigationState.state = NavigationStateIdle();
            return;
        }

        moveTo(unitInfo, *resolvedGoal);
    }

    bool followPath(UnitInfo unitInfo, UnitPhysicsInfoGround& physics, PathFollowingInfo& path)
    {
        const auto& destination = *path.currentWaypoint;
        SimVector xzPosition(unitInfo.state->position.x, 0_ss, unitInfo.state->position.z);
        SimVector xzDestination(destination.x, 0_ss, destination.z);
        auto distanceSquared = xzPosition.distanceSquared(xzDestination);

        auto isFinalDestination = path.currentWaypoint == (path.path.waypoints.end() - 1);

        if (isFinalDestination)
        {
            if (distanceSquared < (8_ss * 8_ss))
            {
                return true;
            }

            physics.steeringInfo = arrive(*unitInfo.state, *unitInfo.definition, physics, destination);
            return false;
        }

        if (distanceSquared < (16_ss * 16_ss))
        {
            ++path.currentWaypoint;
            return false;
        }

        physics.steeringInfo = seek(*unitInfo.state, *unitInfo.definition, destination);
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

        auto direction = match(
            weaponDefinition.physicsType,
            [&](const ProjectilePhysicsTypeLineOfSight&) {
                return (fireInfo->targetPosition - firingPoint).normalized();
            },
            [&](const ProjectilePhysicsTypeTracking&) {
                return (fireInfo->targetPosition - firingPoint).normalized();
            },
            [&](const ProjectilePhysicsTypeBallistic&) {
                return toDirection(fireInfo->heading + unit.rotation, -fireInfo->pitch);
            });


        if (weaponDefinition.sprayAngle != SimAngle(0))
        {
            direction = changeDirectionByRandomAngle(direction, weaponDefinition.sprayAngle);
        }

        auto targetUnit = std::get_if<UnitId>(&attackInfo->target);
        auto targetUnitOption = targetUnit == nullptr ? std::optional<UnitId>() : std::make_optional(*targetUnit);
        sim->spawnProjectile(unit.owner, *weapon, firingPoint, direction, (fireInfo->targetPosition - firingPoint).length(), targetUnitOption);

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

    void UnitBehaviorService::applyUnitSteering(UnitInfo unitInfo)
    {
        updateUnitRotation(unitInfo);
        updateUnitSpeed(unitInfo);
    }

    void UnitBehaviorService::updateUnitRotation(UnitInfo unitInfo)
    {
        auto turnRateThisFrame = SimAngle(unitInfo.definition->turnRate.value);
        unitInfo.state->previousRotation = unitInfo.state->rotation;

        match(
            unitInfo.state->physics,
            [&](const UnitPhysicsInfoGround& p) {
                unitInfo.state->rotation = turnTowards(unitInfo.state->rotation, p.steeringInfo.targetAngle, turnRateThisFrame);
            },
            [&](const UnitPhysicsInfoAir& p) {
                match(
                    p.movementState,
                    [&](const AirMovementStateTakingOff&) {
                        // do nothing
                    },
                    [&](const AirMovementStateLanding&) {
                        // do nothing
                    },
                    [&](const AirMovementStateFlying& m) {
                        if (!m.targetPosition)
                        {
                            // keep rotation as-is if not trying to go anywhere
                            return;
                        }
                        auto direction = *m.targetPosition - unitInfo.state->position;
                        auto targetAngle = UnitState::toRotation(direction);
                        unitInfo.state->rotation = turnTowards(unitInfo.state->rotation, targetAngle, turnRateThisFrame);
                    });
            });
    }

    void UnitBehaviorService::updateUnitSpeed(UnitInfo unitInfo)
    {
        match(
            unitInfo.state->physics,
            [&](UnitPhysicsInfoGround& p) {
                p.currentSpeed = computeNewGroundUnitSpeed(sim->terrain, *unitInfo.state, *unitInfo.definition, p);
            },
            [&](UnitPhysicsInfoAir& p) {
                match(
                    p.movementState,
                    [&](AirMovementStateFlying& m) {
                        m.currentVelocity = computeNewAirUnitVelocity(*unitInfo.state, *unitInfo.definition, m);
                    },
                    [&](const AirMovementStateTakingOff&) {
                        // do nothing
                    },
                    [&](const AirMovementStateLanding&) {
                        // do nothing
                    });
            });
    }

    void UnitBehaviorService::updateGroundUnitPosition(UnitInfo unitInfo, const UnitPhysicsInfoGround& physics)
    {
        auto direction = UnitState::toDirection(unitInfo.state->rotation);

        if (physics.currentSpeed > 0_ss)
        {
            auto newPosition = unitInfo.state->position + (direction * physics.currentSpeed);
            newPosition.y = sim->terrain.getHeightAt(newPosition.x, newPosition.z);
            if (unitInfo.definition->floater || unitInfo.definition->canHover)
            {
                newPosition.y = rweMax(newPosition.y, sim->terrain.getSeaLevel());
            }

            if (!tryApplyMovementToPosition(unitInfo, newPosition))
            {
                unitInfo.state->inCollision = true;

                // if we failed to move, try in each axis separately
                // to see if we can complete a "partial" movement
                const SimVector maskX(0_ss, 1_ss, 1_ss);
                const SimVector maskZ(1_ss, 1_ss, 0_ss);

                SimVector newPos1;
                SimVector newPos2;
                if (direction.x > direction.z)
                {
                    newPos1 = unitInfo.state->position + (direction * maskZ * physics.currentSpeed);
                    newPos2 = unitInfo.state->position + (direction * maskX * physics.currentSpeed);
                }
                else
                {
                    newPos1 = unitInfo.state->position + (direction * maskX * physics.currentSpeed);
                    newPos2 = unitInfo.state->position + (direction * maskZ * physics.currentSpeed);
                }
                newPos1.y = sim->terrain.getHeightAt(newPos1.x, newPos1.z);
                newPos2.y = sim->terrain.getHeightAt(newPos2.x, newPos2.z);

                if (unitInfo.definition->floater || unitInfo.definition->canHover)
                {
                    newPos1.y = rweMax(newPos1.y, sim->terrain.getSeaLevel());
                    newPos2.y = rweMax(newPos2.y, sim->terrain.getSeaLevel());
                }

                if (!tryApplyMovementToPosition(unitInfo, newPos1))
                {
                    tryApplyMovementToPosition(unitInfo, newPos2);
                }
            }
        }
    }

    void UnitBehaviorService::updateUnitPosition(UnitInfo unitInfo)
    {
        unitInfo.state->previousPosition = unitInfo.state->position;
        unitInfo.state->inCollision = false;

        match(
            unitInfo.state->physics,
            [&](const UnitPhysicsInfoGround& p) {
                updateGroundUnitPosition(unitInfo, p);
            },
            [&](const UnitPhysicsInfoAir& p) {
                match(
                    p.movementState,
                    [&](const AirMovementStateFlying& m) {
                        auto newPosition = unitInfo.state->position + m.currentVelocity;
                        tryApplyMovementToPosition(unitInfo, newPosition);
                    },
                    [&](const AirMovementStateTakingOff&) {
                        climbToCruiseAltitude(unitInfo);
                    },
                    [&](const AirMovementStateLanding&) {
                        descendToGroundLevel(unitInfo);
                    });
            });
    }

    bool UnitBehaviorService::tryApplyMovementToPosition(UnitInfo unitInfo, const SimVector& newPosition)
    {
        // No collision for flying units.
        if (isFlying(unitInfo.state->physics))
        {
            unitInfo.state->position = newPosition;
            return true;
        }

        // check for collision at the new position
        auto newFootprintRegion = sim->computeFootprintRegion(newPosition, unitInfo.definition->movementCollisionInfo);

        if (sim->isCollisionAt(newFootprintRegion, unitInfo.id))
        {
            return false;
        }

        // Unlike for pathfinding, TA doesn't care about the unit's actual movement class for collision checks,
        // it only cares about the attributes defined directly on the unitInfo.state->
        // Jam these into an ad-hoc movement class to pass into our walkability check.
        if (!isGridPointWalkable(sim->terrain, sim->getAdHocMovementClass(unitInfo.definition->movementCollisionInfo), newFootprintRegion.x, newFootprintRegion.y))
        {
            return false;
        }

        // we passed all collision checks, update accordingly
        auto footprintRegion = sim->computeFootprintRegion(unitInfo.state->position, unitInfo.definition->movementCollisionInfo);
        sim->moveUnitOccupiedArea(footprintRegion, newFootprintRegion, unitInfo.id);

        auto seaLevel = sim->terrain.getSeaLevel();
        auto oldTerrainHeight = sim->terrain.getHeightAt(unitInfo.state->position.x, unitInfo.state->position.z);
        auto oldPosBelowSea = oldTerrainHeight < seaLevel;

        unitInfo.state->position = newPosition;

        auto newTerrainHeight = sim->terrain.getHeightAt(unitInfo.state->position.x, unitInfo.state->position.z);
        auto newPosBelowSea = newTerrainHeight < seaLevel;

        if (oldPosBelowSea && !newPosBelowSea)
        {
            unitInfo.state->cobEnvironment->createThread("setSFXoccupy", std::vector<int>{4});
        }
        else if (!oldPosBelowSea && newPosBelowSea)
        {
            unitInfo.state->cobEnvironment->createThread("setSFXoccupy", std::vector<int>{2});
        }

        return true;
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

    bool UnitBehaviorService::handleOrder(UnitInfo unitInfo, const UnitOrder& order)
    {
        return match(
            order,
            [&](const MoveOrder& o) {
                return handleMoveOrder(unitInfo, o);
            },
            [&](const AttackOrder& o) {
                return handleAttackOrder(unitInfo, o);
            },
            [&](const BuildOrder& o) {
                return handleBuildOrder(unitInfo, o);
            },
            [&](const BuggerOffOrder& o) {
                return handleBuggerOffOrder(unitInfo, o);
            },
            [&](const CompleteBuildOrder& o) {
                return handleCompleteBuildOrder(unitInfo, o);
            },
            [&](const GuardOrder& o) {
                return handleGuardOrder(unitInfo, o);
            });
    }

    bool UnitBehaviorService::handleMoveOrder(UnitInfo unitInfo, const MoveOrder& moveOrder)
    {
        if (!unitInfo.definition->isMobile)
        {
            return false;
        }

        if (navigateTo(unitInfo, moveOrder.destination))
        {
            sim->events.push_back(UnitArrivedEvent{unitInfo.id});
            return true;
        }

        return false;
    }

    bool UnitBehaviorService::handleAttackOrder(UnitInfo unitInfo, const AttackOrder& attackOrder)
    {
        return attackTarget(unitInfo, attackOrder.target);
    }

    bool UnitBehaviorService::attackTarget(UnitInfo unitInfo, const AttackTarget& target)
    {
        if (!unitInfo.state->weapons[0])
        {
            return true;
        }

        const auto& weaponDefinition = sim->weaponDefinitions.at(unitInfo.state->weapons[0]->weaponType);

        auto targetPosition = getTargetPosition(target);
        if (!targetPosition)
        {
            // target has gone away, throw away this order
            return true;
        }

        auto maxRangeSquared = weaponDefinition.maxRange * weaponDefinition.maxRange;
        if (unitInfo.state->position.distanceSquared(*targetPosition) > maxRangeSquared)
        {
            navigateTo(unitInfo, *targetPosition);
        }
        else
        {
            // we're in range, aim weapons
            for (unsigned int i = 0; i < 2; ++i)
            {
                match(
                    target,
                    [&](const UnitId& u) { unitInfo.state->setWeaponTarget(i, u); },
                    [&](const SimVector& v) { unitInfo.state->setWeaponTarget(i, v); });
            }
        }

        return false;
    }

    bool UnitBehaviorService::handleBuildOrder(UnitInfo unitInfo, const BuildOrder& buildOrder)
    {
        return buildUnit(unitInfo, buildOrder.unitType, buildOrder.position);
    }

    bool UnitBehaviorService::handleBuggerOffOrder(UnitInfo unitInfo, const BuggerOffOrder& buggerOffOrder)
    {
        auto [footprintX, footprintZ] = sim->getFootprintXZ(unitInfo.definition->movementCollisionInfo);
        return navigateTo(unitInfo, buggerOffOrder.rect.expand((footprintX * 3) - 4, (footprintZ * 3) - 4));
    }

    bool UnitBehaviorService::handleCompleteBuildOrder(UnitInfo unitInfo, const rwe::CompleteBuildOrder& buildOrder)
    {
        return buildExistingUnit(unitInfo, buildOrder.target);
    }

    bool UnitBehaviorService::handleGuardOrder(UnitInfo unitInfo, const GuardOrder& guardOrder)
    {
        auto target = sim->tryGetUnitState(guardOrder.target);
        // TODO: real allied check here
        if (!target || !target->get().isOwnedBy(unitInfo.state->owner))
        {
            // unit is dead or a traitor, abandon order
            return true;
        }
        auto& targetUnit = target->get();


        // assist building
        if (auto bs = std::get_if<UnitBehaviorStateBuilding>(&targetUnit.behaviourState); unitInfo.definition->builder && bs)
        {
            buildExistingUnit(unitInfo, bs->targetUnit);
            return false;
        }

        // assist factory building
        if (auto fs = std::get_if<FactoryBehaviorStateBuilding>(&targetUnit.factoryState); unitInfo.definition->builder && fs)
        {
            if (fs->targetUnit)
            {
                buildExistingUnit(unitInfo, fs->targetUnit->first);
                return false;
            }
        }

        // stay close
        if (unitInfo.definition->canMove && unitInfo.state->position.distanceSquared(targetUnit.position) > SimScalar(200 * 200))
        {
            navigateTo(unitInfo, targetUnit.position);
            return false;
        }

        return false;
    }

    bool UnitBehaviorService::handleBuild(UnitInfo unitInfo, const std::string& unitType)
    {
        return match(
            unitInfo.state->factoryState,
            [&](const FactoryBehaviorStateIdle&) {
                sim->activateUnit(unitInfo.id);
                unitInfo.state->factoryState = FactoryBehaviorStateBuilding();
                return false;
            },
            [&](FactoryBehaviorStateCreatingUnit& state) {
                return match(
                    state.status,
                    [&](const UnitCreationStatusPending&) {
                        return false;
                    },
                    [&](const UnitCreationStatusDone& s) {
                        unitInfo.state->cobEnvironment->createThread("StartBuilding");
                        unitInfo.state->factoryState = FactoryBehaviorStateBuilding{std::make_pair(s.unitId, std::optional<SimVector>())};
                        return false;
                    },
                    [&](const UnitCreationStatusFailed&) {
                        unitInfo.state->factoryState = FactoryBehaviorStateBuilding();
                        return false;
                    });
            },
            [&](FactoryBehaviorStateBuilding& state) {
                if (!unitInfo.state->inBuildStance)
                {
                    return false;
                }

                auto buildPieceInfo = getBuildPieceInfo(unitInfo.id);
                // buildPieceInfo.position.y = sim->terrain.getHeightAt(buildPieceInfo.position.x, buildPieceInfo.position.z);
                if (!state.targetUnit)
                {
                    unitInfo.state->factoryState = FactoryBehaviorStateCreatingUnit{unitType, unitInfo.state->owner, buildPieceInfo.position, buildPieceInfo.rotation};
                    sim->unitCreationRequests.push_back(unitInfo.id);
                    return false;
                }

                auto targetUnitOption = sim->tryGetUnitState(state.targetUnit->first);
                if (!targetUnitOption)
                {
                    unitInfo.state->factoryState = FactoryBehaviorStateCreatingUnit{unitType, unitInfo.state->owner, buildPieceInfo.position, buildPieceInfo.rotation};
                    sim->unitCreationRequests.push_back(unitInfo.id);
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
                    unitInfo.state->cobEnvironment->createThread("StopBuilding");
                    sim->deactivateUnit(unitInfo.id);
                    unitInfo.state->factoryState = FactoryBehaviorStateIdle();
                    return true;
                }

                if (!targetUnit.isBeingBuilt(targetUnitDefinition))
                {
                    if (unitInfo.state->orders.empty())
                    {
                        auto footprintRect = sim->computeFootprintRegion(unitInfo.state->position, unitInfo.definition->movementCollisionInfo);
                        targetUnit.addOrder(BuggerOffOrder(footprintRect));
                    }
                    else
                    {
                        targetUnit.replaceOrders(unitInfo.state->orders);
                    }
                    unitInfo.state->cobEnvironment->createThread("StopBuilding");
                    sim->deactivateUnit(unitInfo.id);
                    unitInfo.state->factoryState = FactoryBehaviorStateIdle();
                    return true;
                }

                if (targetUnitDefinition.floater || targetUnitDefinition.canHover)
                {
                    buildPieceInfo.position.y = rweMax(buildPieceInfo.position.y, sim->terrain.getSeaLevel());
                }

                tryApplyMovementToPosition(sim->getUnitInfo(state.targetUnit->first), buildPieceInfo.position);
                targetUnit.rotation = buildPieceInfo.rotation;

                auto costs = targetUnit.getBuildCostInfo(targetUnitDefinition, unitInfo.definition->workerTimePerTick);
                auto gotResources = sim->addResourceDelta(
                    unitInfo.id,
                    -Energy(targetUnitDefinition.buildCostEnergy.value * static_cast<float>(unitInfo.definition->workerTimePerTick) / static_cast<float>(targetUnitDefinition.buildTime)),
                    -Metal(targetUnitDefinition.buildCostMetal.value * static_cast<float>(unitInfo.definition->workerTimePerTick) / static_cast<float>(targetUnitDefinition.buildTime)),
                    -costs.energyCost,
                    -costs.metalCost);

                if (!gotResources)
                {
                    // we don't have resources available to build -- wait
                    state.targetUnit->second = std::nullopt;
                    return false;
                }
                state.targetUnit->second = getNanoPoint(unitInfo.id);

                if (targetUnit.addBuildProgress(targetUnitDefinition, unitInfo.definition->workerTimePerTick))
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

    void UnitBehaviorService::clearBuild(UnitInfo unitInfo)
    {
        match(
            unitInfo.state->factoryState,
            [&](const FactoryBehaviorStateIdle&) {
                // do nothing
            },
            [&](const FactoryBehaviorStateCreatingUnit& state) {
                match(
                    state.status,
                    [&](const UnitCreationStatusDone& d) {
                        sim->quietlyKillUnit(d.unitId);
                    },
                    [&](const auto&) {
                        // do nothing
                    });
                sim->deactivateUnit(unitInfo.id);
                unitInfo.state->factoryState = FactoryBehaviorStateIdle();
            },
            [&](const FactoryBehaviorStateBuilding& state) {
                if (state.targetUnit)
                {
                    sim->quietlyKillUnit(state.targetUnit->first);
                    unitInfo.state->cobEnvironment->createThread("StopBuilding");
                }
                sim->deactivateUnit(unitInfo.id);
                unitInfo.state->factoryState = FactoryBehaviorStateIdle();
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
            [](const SimVector& v) { return std::make_optional(v); },
            [this](UnitId id) { return tryGetSweetSpot(id); });
    }

    bool UnitBehaviorService::groundUnitMoveTo(UnitInfo unitInfo, const MovingStateGoal& goal)
    {
        auto movingState = std::get_if<NavigationStateMoving>(&unitInfo.state->navigationState.state);

        if (!movingState || movingState->destination != goal)
        {
            // request a path to follow
            unitInfo.state->navigationState.state = NavigationStateMoving{goal, std::nullopt, true};
            sim->requestPath(unitInfo.id);
            return false;
        }

        // if we are colliding, request a new path
        if (unitInfo.state->inCollision && !movingState->pathRequested)
        {
            // only request a new path if we don't have one yet,
            // or we've already had our current one for a bit
            if (!movingState->path || (sim->gameTime - movingState->path->pathCreationTime) >= GameTime(30))
            {
                sim->requestPath(unitInfo.id);
                movingState->pathRequested = true;
            }
        }

        // if a path is available, attempt to follow it
        if (movingState->path)
        {
            auto groundPhysics = std::get_if<UnitPhysicsInfoGround>(&unitInfo.state->physics);
            if (groundPhysics == nullptr)
            {
                throw std::logic_error("ground unit does not have ground physics");
            }
            if (followPath(unitInfo, *groundPhysics, *movingState->path))
            {
                // we finished following the path
                return true;
            }
        }

        return false;
    }

    bool UnitBehaviorService::flyingUnitMoveTo(UnitInfo unitInfo, const MovingStateGoal& goal)
    {
        return match(
            unitInfo.state->physics,
            [&](UnitPhysicsInfoGround& p) {
                p.steeringInfo.shouldTakeOff = true;
                return false;
            },
            [&](const UnitPhysicsInfoAir& p) {
                return flyTowardsGoal(unitInfo, goal);
            });
    }

    bool UnitBehaviorService::navigateTo(UnitInfo unitInfo, const NavigationGoal& goal)
    {
        unitInfo.state->navigationState.desiredDestination = goal;

        return hasReachedGoal(*sim, sim->terrain, *unitInfo.state, *unitInfo.definition, goal);
    }

    bool UnitBehaviorService::moveTo(UnitInfo unitInfo, const MovingStateGoal& goal)
    {
        if (unitInfo.definition->canFly)
        {
            return flyingUnitMoveTo(unitInfo, goal);
        }
        else
        {
            return groundUnitMoveTo(unitInfo, goal);
        }
    }

    UnitCreationStatus UnitBehaviorService::createNewUnit(UnitInfo unitInfo, const std::string& unitType, const SimVector& position)
    {
        if (auto s = std::get_if<UnitBehaviorStateCreatingUnit>(&unitInfo.state->behaviourState))
        {
            if (s->unitType == unitType && s->position == position)
            {
                return s->status;
            }
        }

        const auto& targetUnitDefinition = sim->unitDefinitions.at(unitType);
        auto footprintRect = sim->computeFootprintRegion(position, targetUnitDefinition.movementCollisionInfo);
        if (navigateTo(unitInfo, footprintRect))
        {
            // TODO: add an additional distance check here -- we may have done the best
            // we can to move but been prevented by some obstacle, so we are too far away still.
            changeState(*unitInfo.state, UnitBehaviorStateCreatingUnit{unitType, unitInfo.state->owner, position});
            sim->unitCreationRequests.push_back(unitInfo.id);
        }

        return UnitCreationStatusPending();
    }

    bool UnitBehaviorService::buildUnit(UnitInfo unitInfo, const std::string& unitType, const SimVector& position)
    {
        auto& unit = sim->getUnitState(unitInfo.id);
        if (!unit.buildOrderUnitId)
        {
            auto result = createNewUnit(unitInfo, unitType, position);
            return match(
                result,
                [&](const UnitCreationStatusPending&) { return false; },
                [&](const UnitCreationStatusFailed&) { return true; },
                [&](const UnitCreationStatusDone& d) {
                    unit.buildOrderUnitId = d.unitId;
                    return deployBuildArm(unitInfo, d.unitId);
                });
        }

        return deployBuildArm(unitInfo, *unit.buildOrderUnitId);
    }

    bool UnitBehaviorService::buildExistingUnit(UnitInfo unitInfo, UnitId targetUnitId)
    {
        auto targetUnitRef = sim->tryGetUnitState(targetUnitId);

        if (!targetUnitRef || targetUnitRef->get().isDead() || !targetUnitRef->get().isBeingBuilt(*unitInfo.definition))
        {
            changeState(*unitInfo.state, UnitBehaviorStateIdle());
            return true;
        }
        auto& targetUnit = targetUnitRef->get();

        // FIXME: this distance measure is wrong
        // Experiment has shown that the distance from which a new building
        // can be started (when caged in) is greater than assist distance,
        // and it appears both measures something more advanced than center <-> center distance.
        if (unitInfo.state->position.distanceSquared(targetUnit.position) > (unitInfo.definition->buildDistance * unitInfo.definition->buildDistance))
        {
            navigateTo(unitInfo, targetUnit.position);
            return false;
        }

        // we're close enough -- actually build the unit
        return deployBuildArm(unitInfo, targetUnitId);
    }

    void UnitBehaviorService::changeState(UnitState& unit, const UnitBehaviorState& newState)
    {
        if (std::holds_alternative<UnitBehaviorStateBuilding>(unit.behaviourState))
        {
            unit.cobEnvironment->createThread("StopBuilding");
        }
        unit.behaviourState = newState;
    }
    bool UnitBehaviorService::deployBuildArm(UnitInfo unitInfo, UnitId targetUnitId)
    {
        auto targetUnitRef = sim->tryGetUnitState(targetUnitId);
        if (!targetUnitRef || targetUnitRef->get().isDead() || !targetUnitRef->get().isBeingBuilt(sim->unitDefinitions.at(targetUnitRef->get().unitType)))
        {
            changeState(*unitInfo.state, UnitBehaviorStateIdle());
            return true;
        }
        auto& targetUnit = targetUnitRef->get();
        const auto& targetUnitDefinition = sim->unitDefinitions.at(targetUnit.unitType);

        return match(
            unitInfo.state->behaviourState,
            [&](UnitBehaviorStateBuilding& buildingState) {
                if (targetUnitId != buildingState.targetUnit)
                {
                    changeState(*unitInfo.state, UnitBehaviorStateIdle());
                    return buildExistingUnit(unitInfo, targetUnitId);
                }

                if (!unitInfo.state->inBuildStance)
                {
                    // We are not in the correct stance to build the unit yet, wait.
                    return false;
                }

                auto costs = targetUnit.getBuildCostInfo(targetUnitDefinition, unitInfo.definition->workerTimePerTick);
                auto gotResources = sim->addResourceDelta(
                    unitInfo.id,
                    -Energy(targetUnitDefinition.buildCostEnergy.value * static_cast<float>(unitInfo.definition->workerTimePerTick) / static_cast<float>(targetUnitDefinition.buildTime)),
                    -Metal(targetUnitDefinition.buildCostMetal.value * static_cast<float>(unitInfo.definition->workerTimePerTick) / static_cast<float>(targetUnitDefinition.buildTime)),
                    -costs.energyCost,
                    -costs.metalCost);

                if (!gotResources)
                {
                    // we don't have resources available to build -- wait
                    buildingState.nanoParticleOrigin = std::nullopt;
                    return false;
                }
                buildingState.nanoParticleOrigin = getNanoPoint(unitInfo.id);

                if (targetUnit.addBuildProgress(targetUnitDefinition, unitInfo.definition->workerTimePerTick))
                {
                    sim->events.push_back(UnitCompleteEvent{buildingState.targetUnit});

                    if (targetUnitDefinition.activateWhenBuilt)
                    {
                        sim->activateUnit(buildingState.targetUnit);
                    }

                    changeState(*unitInfo.state, UnitBehaviorStateIdle());
                    return true;
                }
                return false;
            },
            [&](const auto&) {
                auto nanoFromPosition = getNanoPoint(unitInfo.id);
                auto headingAndPitch = computeLineOfSightHeadingAndPitch(unitInfo.state->rotation, nanoFromPosition, targetUnit.position);
                auto heading = headingAndPitch.first;
                auto pitch = headingAndPitch.second;

                changeState(*unitInfo.state, UnitBehaviorStateBuilding{targetUnitId, std::nullopt});
                unitInfo.state->cobEnvironment->createThread("StartBuilding", {toCobAngle(heading).value, toCobAngle(pitch).value});
                return false;
            });
    }

    bool UnitBehaviorService::climbToCruiseAltitude(UnitInfo unitInfo)
    {
        auto targetHeight = getTargetAltitude(sim->terrain, unitInfo.state->position.x, unitInfo.state->position.z, *unitInfo.definition);

        unitInfo.state->position.y = rweMin(unitInfo.state->position.y + 1_ss, targetHeight);

        return unitInfo.state->position.y == targetHeight;
    }

    bool UnitBehaviorService::descendToGroundLevel(UnitInfo unitInfo)
    {
        auto terrainHeight = sim->terrain.getHeightAt(unitInfo.state->position.x, unitInfo.state->position.z);

        unitInfo.state->position.y = rweMax(unitInfo.state->position.y - 1_ss, terrainHeight);

        return unitInfo.state->position.y == terrainHeight;
    }

    void UnitBehaviorService::transitionFromGroundToAir(UnitInfo unitInfo)
    {
        unitInfo.state->activate();

        unitInfo.state->physics = UnitPhysicsInfoAir();
        auto footprintRect = sim->computeFootprintRegion(unitInfo.state->position, unitInfo.definition->movementCollisionInfo);
        auto footprintRegion = sim->occupiedGrid.tryToRegion(footprintRect);
        assert(!!footprintRegion);
        sim->occupiedGrid.forEach(*footprintRegion, [](auto& cell) {
            cell.occupiedType = OccupiedNone();
        });
        sim->flyingUnitsSet.insert(unitInfo.id);
    }

    bool UnitBehaviorService::tryTransitionFromAirToGround(UnitInfo unitInfo)
    {
        auto footprintRect = sim->computeFootprintRegion(unitInfo.state->position, unitInfo.definition->movementCollisionInfo);
        auto footprintRegion = sim->occupiedGrid.tryToRegion(footprintRect);
        assert(!!footprintRegion);

        if (sim->isCollisionAt(*footprintRegion))
        {
            return false;
        }

        sim->occupiedGrid.forEach(*footprintRegion, [&](auto& cell) {
            cell.occupiedType = OccupiedUnit(unitInfo.id);
        });
        sim->flyingUnitsSet.erase(unitInfo.id);

        unitInfo.state->physics = UnitPhysicsInfoGround();

        return true;
    }

    bool UnitBehaviorService::flyTowardsGoal(UnitInfo unitInfo, const MovingStateGoal& goal)
    {
        auto destination = match(
            goal,
            [&](const SimVector& pos) {
                return pos;
            },
            [&](const DiscreteRect& rect) {
                return findClosestPointToFootprintXZ(sim->terrain, rect, unitInfo.state->position);
            });

        SimVector xzPosition(unitInfo.state->position.x, 0_ss, unitInfo.state->position.z);
        SimVector xzDestination(destination.x, 0_ss, destination.z);
        auto distanceSquared = xzPosition.distanceSquared(xzDestination);

        if (distanceSquared < (8_ss * 8_ss))
        {
            return true;
        }

        auto airPhysics = std::get_if<UnitPhysicsInfoAir>(&unitInfo.state->physics);
        if (airPhysics == nullptr)
        {
            throw std::logic_error("cannot fly towards goal because unit does not have air physics");
        }

        match(
            airPhysics->movementState,
            [&](AirMovementStateFlying& m) {
                auto targetHeight = getTargetAltitude(sim->terrain, destination.x, destination.z, *unitInfo.definition);
                SimVector destinationAtAltitude(destination.x, targetHeight, destination.z);

                m.targetPosition = destinationAtAltitude;
            },
            [&](const AirMovementStateTakingOff&) {
                // do nothing
            },
            [&](AirMovementStateLanding& m) {
                m.shouldAbort = true;
            });

        return false;
    }
}
