#include "UnitBehaviorService.h"
#include <rwe/GameScene.h>
#include <rwe/Index.h>
#include <rwe/cob/CobExecutionContext.h>
#include <rwe/geometry/Circle2x.h>
#include <rwe/match.h>
#include <rwe/math/rwe_math.h>
#include <rwe/sim/movement.h>

namespace rwe
{
    Vector2x<SimScalar> Vector2xFromLengthAndAngle(SimScalar length, SimAngle angle)
    {
        return Vector2x<SimScalar>(sin(angle), cos(angle)) * length;
    }

    bool isWithinTurningCircle(const SimVector& dest, SimScalar speed, SimScalar turnRate, SimAngle currentDirection)
    {
        auto turnRadius = speed / angularToRadians(turnRate);

        auto anticlockwiseCircleAngle = currentDirection + QuarterTurn;
        auto clockwiseCircleAngle = currentDirection - QuarterTurn;
        auto anticlockwiseCircle = Circle2x<SimScalar>(turnRadius, Vector2xFromLengthAndAngle(turnRadius, anticlockwiseCircleAngle));
        auto clockwiseCircle = Circle2x<SimScalar>(turnRadius, Vector2xFromLengthAndAngle(turnRadius, clockwiseCircleAngle));

        return anticlockwiseCircle.contains(dest.xz()) || clockwiseCircle.contains(dest.xz());
    }

    UnitBehaviorService::UnitBehaviorService(
        GameScene* scene,
        UnitFactory* unitFactory,
        CobExecutionService* cobExecutionService)
        : scene(scene), unitFactory(unitFactory), cobExecutionService(cobExecutionService)
    {
    }

    void UnitBehaviorService::onCreate(UnitId unitId)
    {
        auto& sim = scene->getSimulation();
        auto& unit = sim.getUnit(unitId);

        unit.cobEnvironment->createThread("Create", std::vector<int>());

        // set speed for metal extractors
        if (unit.extractsMetal != Metal(0))
        {
            auto footprint = scene->computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
            auto metalValue = sim.metalGrid.accumulate(sim.metalGrid.clipRegion(footprint), 0u, std::plus<>());
            unit.cobEnvironment->createThread("SetSpeed", {static_cast<int>(metalValue)});
        }

        cobExecutionService->run(*scene, scene->getSimulation(), unitId);

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
                    unit.buildOrderUnitId = std::nullopt;
                }
            }
            else
            {
                changeState(unit, IdleState());
            }

            for (Index i = 0; i < getSize(unit.weapons); ++i)
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
            aimVector = Unit::toDirection(rotation);
        }

        SimVector aimVectorXZ(aimVector.x, 0_ss, aimVector.z);

        auto heading = Unit::toRotation(aimVectorXZ);
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
            aimVector = Unit::toDirection(rotation);
        }

        SimVector aimVectorXZ(aimVector.x, 0_ss, aimVector.z);

        auto heading = Unit::toRotation(aimVectorXZ);
        heading = heading - rotation;

        auto pitches = computeFiringAngles(speed, gravity, aimVectorXZ.length() - zOffset, aimVector.y);
        if (!pitches)
        {
            return {heading, EighthTurn};
        }

        return {heading, pitches->second};
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

        const auto& weaponDefinition = scene->getSimulation().weaponDefinitions.at(weapon->weaponType);

        if (auto idleState = std::get_if<UnitWeaponStateIdle>(&weapon->state); idleState != nullptr)
        {
            // attempt to acquire a target
            if (!weaponDefinition.commandFire && unit.fireOrders == UnitFireOrders::FireAtWill)
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
        auto& rng = scene->getSimulation().rng;
        auto angle = SimAngle(dist(rng));
        if (dist2(rng))
        {
            angle = SimAngle(0) - angle;
        }

        return rotateDirectionXZ(direction, angle);
    }

    void UnitBehaviorService::tryFireWeapon(UnitId id, unsigned int weaponIndex)
    {
        auto& unit = scene->getSimulation().getUnit(id);
        auto& weapon = unit.weapons[weaponIndex];

        if (!weapon)
        {
            return;
        }

        const auto& weaponDefinition = scene->getSimulation().weaponDefinitions.at(weapon->weaponType);
        const auto& weaponMediaInfo = scene->getMeshDatabase().getWeapon(weapon->weaponType);

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
        auto gameTime = scene->getGameTime();
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

        scene->getSimulation().spawnProjectile(unit.owner, *weapon, firingPoint, direction, (fireInfo->targetPosition - firingPoint).length());

        if (fireInfo->burstsFired == 0 || weaponMediaInfo.soundTrigger)
        {
            scene->playWeaponStartSound(simVectorToFloat(firingPoint), weapon->weaponType);
        }

        // If we just started the burst, set the reload timer
        if (fireInfo->burstsFired == 0)
        {
            unit.cobEnvironment->createThread(getFireScriptName(weaponIndex));
            weapon->readyTime = gameTime + deltaSecondsToTicks(weaponDefinition.reloadTime);
            if (weaponMediaInfo.startSmoke)
            {
                scene->createWeaponSmoke(simVectorToFloat(firingPoint));
            }
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
        auto& unit = scene->getSimulation().getUnit(id);
        auto turnRateThisFrame = SimAngle(unit.turnRate.value);
        unit.previousRotation = unit.rotation;
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

        unit.previousPosition = unit.position;

        auto direction = Unit::toDirection(unit.rotation);

        unit.inCollision = false;

        if (unit.currentSpeed > 0_ss)
        {
            auto newPosition = unit.position + (direction * unit.currentSpeed);
            newPosition.y = scene->getTerrain().getHeightAt(newPosition.x, newPosition.z);
            if (unit.floater || unit.canHover)
            {
                newPosition.y = rweMax(newPosition.y, scene->getTerrain().getSeaLevel());
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
                newPos1.y = scene->getTerrain().getHeightAt(newPos1.x, newPos1.z);
                newPos2.y = scene->getTerrain().getHeightAt(newPos2.x, newPos2.z);

                if (unit.floater || unit.canHover)
                {
                    newPos1.y = rweMax(newPos1.y, scene->getTerrain().getSeaLevel());
                    newPos2.y = rweMax(newPos2.y, scene->getTerrain().getSeaLevel());
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
        auto& sim = scene->getSimulation();
        auto& unit = sim.getUnit(id);

        // check for collision at the new position
        auto newFootprintRegion = scene->computeFootprintRegion(newPosition, unit.footprintX, unit.footprintZ);

        if (scene->isCollisionAt(newFootprintRegion, id))
        {
            return false;
        }

        // Unlike for pathfinding, TA doesn't care about the unit's actual movement class for collision checks,
        // it only cares about the attributes defined directly on the unit.
        // Jam these into an ad-hoc movement class to pass into our walkability check.
        if (!isGridPointWalkable(sim.terrain, unit.getAdHocMovementClass(), newFootprintRegion.x, newFootprintRegion.y))
        {
            return false;
        }

        // we passed all collision checks, update accordingly
        auto footprintRegion = scene->computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        scene->moveUnitOccupiedArea(footprintRegion, newFootprintRegion, id);

        auto seaLevel = scene->getTerrain().getSeaLevel();
        auto oldTerrainHeight = scene->getTerrain().getHeightAt(unit.position.x, unit.position.z);
        auto oldPosBelowSea = oldTerrainHeight < seaLevel;

        unit.position = newPosition;

        auto newTerrainHeight = scene->getTerrain().getHeightAt(unit.position.x, unit.position.z);
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
        auto& unit = scene->getSimulation().getUnit(id);
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
        const auto& unit = scene->getSimulation().getUnit(id);
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
        const auto& unit = scene->getSimulation().getUnit(id);
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
            },
            [this, unitId](const CompleteBuildOrder& o) {
                return handleCompleteBuildOrder(unitId, o);
            },
            [this, unitId](const GuardOrder& o) {
                return handleGuardOrder(unitId, o);
            });
    }

    bool UnitBehaviorService::handleMoveOrder(UnitId unitId, const MoveOrder& moveOrder)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);
        if (!unit.isMobile)
        {
            return false;
        }

        if (moveTo(unitId, moveOrder.destination))
        {
            scene->playUnitNotificationSound(unit.owner, unit.unitType, UnitSoundType::Arrived1);
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
        auto& unit = scene->getSimulation().getUnit(unitId);

        if (!unit.weapons[0])
        {
            return true;
        }

        const auto& weaponDefinition = scene->getSimulation().weaponDefinitions.at(unit.weapons[0]->weaponType);

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
                    [&](const UnitId& u) { unit.setWeaponTarget(i, u); },
                    [&](const SimVector& v) { unit.setWeaponTarget(i, v); });
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
        auto& unit = scene->getSimulation().getUnit(unitId);
        return moveTo(unitId, buggerOffOrder.rect.expand((unit.footprintX * 3) - 4, (unit.footprintZ * 3) - 4));
    }

    bool UnitBehaviorService::handleCompleteBuildOrder(rwe::UnitId unitId, const rwe::CompleteBuildOrder& buildOrder)
    {
        return buildExistingUnit(unitId, buildOrder.target);
    }

    bool UnitBehaviorService::handleGuardOrder(UnitId unitId, const GuardOrder& guardOrder)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        auto target = scene->getSimulation().tryGetUnit(guardOrder.target);
        // TODO: real allied check here
        if (!target || !target->get().isOwnedBy(unit.owner))
        {
            // unit is dead or a traitor, abandon order
            return true;
        }
        auto& targetUnit = target->get();


        // assist building
        if (auto bs = std::get_if<BuildingState>(&targetUnit.behaviourState); unit.builder && bs)
        {
            buildExistingUnit(unitId, bs->targetUnit);
            return false;
        }

        // assist factory building
        if (auto fs = std::get_if<FactoryStateBuilding>(&targetUnit.factoryState); unit.builder && fs)
        {
            if (fs->targetUnit)
            {
                buildExistingUnit(unitId, fs->targetUnit->first);
                return false;
            }
        }

        // stay close
        if (unit.canMove && unit.position.distanceSquared(targetUnit.position) > SimScalar(200 * 200))
        {
            moveTo(unitId, targetUnit.position);
            return false;
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
            [&](FactoryStateCreatingUnit& state) {
                return match(
                    state.status,
                    [&](const UnitCreationStatusPending&) {
                        return false;
                    },
                    [&](const UnitCreationStatusDone& s) {
                        unit.cobEnvironment->createThread("StartBuilding");
                        unit.factoryState = FactoryStateBuilding{std::make_pair(s.unitId, std::optional<SimVector>())};
                        return false;
                    },
                    [&](const UnitCreationStatusFailed&) {
                        unit.factoryState = FactoryStateBuilding();
                        return false;
                    });
            },
            [&](FactoryStateBuilding& state) {
                if (!unit.inBuildStance)
                {
                    return false;
                }

                auto& sim = scene->getSimulation();

                auto buildPieceInfo = getBuildPieceInfo(unitId);
                //buildPieceInfo.position.y = sim.terrain.getHeightAt(buildPieceInfo.position.x, buildPieceInfo.position.z);
                if (!state.targetUnit)
                {
                    unit.factoryState = FactoryStateCreatingUnit{unitType, unit.owner, buildPieceInfo.position, buildPieceInfo.rotation};
                    scene->getSimulation().unitCreationRequests.push_back(unitId);
                    return false;
                }

                auto targetUnitOption = scene->getSimulation().tryGetUnit(state.targetUnit->first);
                if (!targetUnitOption)
                {
                    unit.factoryState = FactoryStateCreatingUnit{unitType, unit.owner, buildPieceInfo.position, buildPieceInfo.rotation};
                    scene->getSimulation().unitCreationRequests.push_back(unitId);
                    return false;
                }

                auto& targetUnit = targetUnitOption->get();

                if (targetUnit.unitType != unitType)
                {
                    if (targetUnit.isBeingBuilt() && !targetUnit.isDead())
                    {
                        scene->quietlyKillUnit(state.targetUnit->first);
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
                    if (unit.orders.empty())
                    {
                        auto footprintRect = scene->computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
                        targetUnit.addOrder(BuggerOffOrder(footprintRect));
                    }
                    else
                    {
                        targetUnit.replaceOrders(unit.orders);
                    }
                    unit.cobEnvironment->createThread("StopBuilding");
                    scene->deactivateUnit(unitId);
                    unit.factoryState = FactoryStateIdle();
                    return true;
                }

                if (targetUnit.floater || targetUnit.canHover)
                {
                    buildPieceInfo.position.y = rweMax(buildPieceInfo.position.y, scene->getTerrain().getSeaLevel());
                }

                tryApplyMovementToPosition(state.targetUnit->first, buildPieceInfo.position);
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
                    state.targetUnit->second = std::nullopt;
                    return false;
                }
                state.targetUnit->second = getNanoPoint(unitId);

                if (targetUnit.addBuildProgress(unit.workerTimePerTick))
                {
                    // play sound when the unit is completed
                    scene->playUnitNotificationSound(targetUnit.owner, targetUnit.unitType, UnitSoundType::UnitComplete);
                    if (targetUnit.activateWhenBuilt)
                    {
                        scene->activateUnit(state.targetUnit->first);
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
            [&](const FactoryStateCreatingUnit& state) {
                match(
                    state.status,
                    [&](const UnitCreationStatusDone& d) {
                        scene->quietlyKillUnit(d.unitId);
                    },
                    [&](const auto&) {
                        // do nothing
                    });
                scene->deactivateUnit(unitId);
                unit.factoryState = FactoryStateIdle();
            },
            [&](const FactoryStateBuilding& state) {
                if (state.targetUnit)
                {
                    scene->quietlyKillUnit(state.targetUnit->first);
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

    SimVector UnitBehaviorService::getPieceLocalPosition(UnitId id, unsigned int pieceId)
    {
        auto& unit = scene->getSimulation().getUnit(id);

        const auto& pieceName = unit.cobEnvironment->_script->pieces.at(pieceId);
        auto pieceTransform = scene->getUnitPieceLocalTransform(id, pieceName);

        return pieceTransform * SimVector(0_ss, 0_ss, 0_ss);
    }

    SimVector UnitBehaviorService::getPiecePosition(UnitId id, unsigned int pieceId)
    {
        auto& unit = scene->getSimulation().getUnit(id);

        return unit.getTransform() * getPieceLocalPosition(id, pieceId);
    }

    SimAngle angleTo(const Vector2x<SimScalar>& lhs, const Vector2x<SimScalar>& rhs)
    {
        return atan2(lhs.det(rhs), lhs.dot(rhs));
    }

    SimAngle UnitBehaviorService::getPieceXZRotation(UnitId id, unsigned int pieceId)
    {
        auto& unit = scene->getSimulation().getUnit(id);

        const auto& pieceName = unit.cobEnvironment->_script->pieces.at(pieceId);
        auto pieceTransform = scene->getUnitPieceLocalTransform(id, pieceName);

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

    bool UnitBehaviorService::moveTo(UnitId unitId, const MovingStateGoal& goal)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        auto movingState = std::get_if<MovingState>(&unit.behaviourState);

        if (!movingState || movingState->destination != goal)
        {
            // request a path to follow
            changeState(unit, MovingState{goal, std::nullopt, true});
            scene->getSimulation().requestPath(unitId);
            return false;
        }

        // if we are colliding, request a new path
        if (unit.inCollision && !movingState->pathRequested)
        {
            auto& sim = scene->getSimulation();

            // only request a new path if we don't have one yet,
            // or we've already had our current one for a bit
            if (!movingState->path || (sim.gameTime - movingState->path->pathCreationTime) >= GameTime(30))
            {
                sim.requestPath(unitId);
                movingState->pathRequested = true;
            }
        }

        // if a path is available, attempt to follow it
        if (movingState->path)
        {
            if (followPath(unit, *movingState->path))
            {
                // we finished following the path,
                // clear our state
                changeState(unit, IdleState());
                return true;
            }
        }

        return false;
    }

    UnitCreationStatus UnitBehaviorService::createNewUnit(UnitId unitId, const std::string& unitType, const SimVector& position)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        if (auto s = std::get_if<CreatingUnitState>(&unit.behaviourState))
        {
            if (s->unitType == unitType && s->position == position)
            {
                return s->status;
            }
        }

        auto footprint = unitFactory->getUnitFootprint(unitType);
        auto footprintRect = scene->computeFootprintRegion(position, footprint.x, footprint.y);
        if (moveTo(unitId, footprintRect))
        {
            // TODO: add an additional distance check here -- we may have done  the best
            // we can to move but been prevented by some obstacle, so we are too far away still.
            changeState(unit, CreatingUnitState{unitType, unit.owner, position});
            scene->getSimulation().unitCreationRequests.push_back(unitId);
        }

        return UnitCreationStatusPending();
    }

    bool UnitBehaviorService::buildUnit(UnitId unitId, const std::string& unitType, const SimVector& position)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);
        if (!unit.buildOrderUnitId)
        {
            auto result = createNewUnit(unitId, unitType, position);
            return match(
                result,
                [&](const UnitCreationStatusPending&) { return false; },
                [&](const UnitCreationStatusFailed&) { return true; },
                [&](const UnitCreationStatusDone& d) {
                    unit.buildOrderUnitId = d.unitId;
                    return deployBuildArm(unitId, d.unitId);
                });
        }

        return deployBuildArm(unitId, *unit.buildOrderUnitId);
    }

    bool UnitBehaviorService::buildExistingUnit(UnitId unitId, UnitId targetUnitId)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);
        auto targetUnitRef = scene->getSimulation().tryGetUnit(targetUnitId);

        if (!targetUnitRef || targetUnitRef->get().isDead() || !targetUnitRef->get().isBeingBuilt())
        {
            changeState(unit, IdleState());
            return true;
        }
        auto& targetUnit = targetUnitRef->get();

        // FIXME: this distance measure is wrong
        // Experiment has shown that the distance from which a new building
        // can be started (when caged in) is greater than assist distance,
        // and it appears both measures something more advanced than center <-> center distance.
        if (unit.position.distanceSquared(targetUnit.position) > (unit.buildDistance * unit.buildDistance))
        {
            moveTo(unitId, targetUnit.position);
            return false;
        }

        // we're close enough -- actually build the unit
        return deployBuildArm(unitId, targetUnitId);
    }

    void UnitBehaviorService::changeState(Unit& unit, const UnitState& newState)
    {
        if (std::holds_alternative<BuildingState>(unit.behaviourState))
        {
            unit.cobEnvironment->createThread("StopBuilding");
        }
        unit.behaviourState = newState;
    }
    bool UnitBehaviorService::deployBuildArm(UnitId unitId, UnitId targetUnitId)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);
        auto targetUnitRef = scene->getSimulation().tryGetUnit(targetUnitId);
        if (!targetUnitRef || targetUnitRef->get().isDead() || !targetUnitRef->get().isBeingBuilt())
        {
            changeState(unit, IdleState());
            return true;
        }
        auto& targetUnit = targetUnitRef->get();

        return match(
            unit.behaviourState,
            [&](BuildingState& buildingState) {
                if (targetUnitId != buildingState.targetUnit)
                {
                    changeState(unit, IdleState());
                    return buildExistingUnit(unitId, targetUnitId);
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
                    buildingState.nanoParticleOrigin = std::nullopt;
                    return false;
                }
                buildingState.nanoParticleOrigin = getNanoPoint(unitId);

                if (targetUnit.addBuildProgress(unit.workerTimePerTick))
                {
                    // play sound when the unit is completed
                    scene->playUnitNotificationSound(targetUnit.owner, targetUnit.unitType, UnitSoundType::UnitComplete);
                    if (targetUnit.activateWhenBuilt)
                    {
                        scene->activateUnit(buildingState.targetUnit);
                    }
                    changeState(unit, IdleState());
                    return true;
                }
                return false;
            },
            [&](const auto&) {
                auto nanoFromPosition = getNanoPoint(unitId);
                auto headingAndPitch = computeLineOfSightHeadingAndPitch(unit.rotation, nanoFromPosition, targetUnit.position);
                auto heading = headingAndPitch.first;
                auto pitch = headingAndPitch.second;

                changeState(unit, BuildingState{targetUnitId, std::nullopt});
                unit.cobEnvironment->createThread("StartBuilding", {toCobAngle(heading).value, toCobAngle(pitch).value});
                return false;
            });
    }
}
