#include "UnitBehaviorService.h"
#include <rwe/GameScene.h>
#include <rwe/geometry/Circle2f.h>
#include <rwe/math/rwe_math.h>

namespace rwe
{
    Vector2f Vector2fFromLengthAndAngle(float length, float angle)
    {
        auto v = Matrix4f::rotationY(angle) * Vector3f(0.0f, 0.0f, -length);
        return Vector2f(v.x, v.z);
    }

    bool isWithinTurningCircle(const Vector2f& dest, float speed, float turnRate, float currentDirection)
    {
        auto turnRadius = speed / turnRate;

        auto anticlockwiseCircleAngle = currentDirection + (Pif / 2.0f);
        auto clockwiseCircleAngle = currentDirection - (Pif / 2.0f);
        auto anticlockwiseCircle = Circle2f(turnRadius, Vector2fFromLengthAndAngle(turnRadius, anticlockwiseCircleAngle));
        auto clockwiseCircle = Circle2f(turnRadius, Vector2fFromLengthAndAngle(turnRadius, clockwiseCircleAngle));

        return anticlockwiseCircle.contains(dest) || clockwiseCircle.contains(dest);
    }

    UnitBehaviorService::UnitBehaviorService(GameScene* scene, PathFindingService* pathFindingService, MovementClassCollisionService* collisionService)
        : scene(scene), pathFindingService(pathFindingService), collisionService(collisionService)
    {
    }

    void UnitBehaviorService::update(UnitId unitId)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        float previousSpeed = unit.currentSpeed;

        // by default, stay put
        float targetSpeed = 0.0f;
        float targetAngle = unit.rotation;

        // check our orders
        if (!unit.orders.empty())
        {
            const auto& order = unit.orders.front();

            // process move orders
            if (auto moveOrder = boost::get<MoveOrder>(&order); moveOrder != nullptr)
            {
                auto idleState = boost::get<IdleState>(&unit.behaviourState);
                auto movingState = boost::get<MovingState>(&unit.behaviourState);
                if (idleState != nullptr)
                {
                    // request a path to follow
                    scene->getSimulation().requestPath(unitId);
                    const auto& destination = moveOrder->destination;
                    unit.behaviourState = MovingState{destination, boost::none, true};
                }
                else if (movingState != nullptr)
                {
                    // if we are colliding, request a new path
                    if (unit.inCollision && !movingState->pathRequested)
                    {
                        auto& sim = scene->getSimulation();

                        // only request a new path if we don't have one yet,
                        // or we've already had our current one for a bit
                        if (!movingState->path || (sim.gameTime - movingState->path->pathCreationTime) >= GameTimeDelta(60))
                        {
                            sim.requestPath(unitId);
                            movingState->pathRequested = true;
                        }
                    }

                    // if a path is available, attempt to follow it
                    auto& pathToFollow = movingState->path;
                    if (pathToFollow)
                    {
                        const auto& destination = *pathToFollow->currentWaypoint;
                        Vector2f xzPosition(unit.position.x, unit.position.z);
                        Vector2f xzDestination(destination.x, destination.z);
                        auto distanceSquared = xzPosition.distanceSquared(xzDestination);

                        auto isFinalDestination = pathToFollow->currentWaypoint == (pathToFollow->path.waypoints.end() - 1);

                        if (distanceSquared < (8.0f * 8.0f))
                        {
                            if (isFinalDestination)
                            {
                                // order complete
                                unit.orders.pop_front();
                                unit.behaviourState = IdleState();

                                if (unit.arrivedSound)
                                {
                                    scene->playSoundOnSelectChannel(*unit.arrivedSound);
                                }
                            }
                            else
                            {
                                ++pathToFollow->currentWaypoint;
                            }
                        }
                        else
                        {
                            // steer towards the goal
                            auto xzDirection = xzDestination - xzPosition;
                            auto destAngle = Vector2f(0.0f, -1.0f).angleTo(xzDirection);
                            targetAngle = (2.0f * Pif) - destAngle; // convert to anticlockwise in our coordinate system

                            // drive at full speed until we need to brake
                            // to turn or to arrive at the goal
                            auto brakingDistance = (unit.currentSpeed * unit.currentSpeed) / (2.0f * unit.brakeRate);

                            if (isWithinTurningCircle(xzDirection, unit.currentSpeed, unit.turnRate, unit.rotation))
                            {
                                targetSpeed = 0.0f;
                            }
                            else if (isFinalDestination && distanceSquared <= (brakingDistance * brakingDistance))
                            {
                                targetSpeed = 0.0f;
                            }
                            else
                            {
                                targetSpeed = unit.maxSpeed;
                            }
                        }
                    }
                }
            }
            else if (auto attackGroundOrder = boost::get<AttackGroundOrder>(&order); attackGroundOrder != nullptr)
            {
                if (unit.weapons.size() > 0)
                {
                    // for now, just assume that we only care about the first weapon
                    // FIXME: all this logic really needs to come out.
                    // Weapons should run their own independent AI logic
                    // (that runs every frame regardless of the unit's order).
                    // Right now it's possible for a weapon to get stuck waiting for
                    // some non-existent aim thread, because orders could change underneath us,
                    // and there's loads of other oddities like not calling TargetCleared reliably.
                    auto& weapon = unit.weapons[0];
                    if (auto idleState = boost::get<UnitWeaponStateIdle>(&weapon.state); idleState != nullptr)
                    {
                        auto aimVector = attackGroundOrder->target - unit.position;
                        auto heading = Vector2f(0.0f, -1.0f).angleTo(Vector2f(aimVector.x, aimVector.z));
                        heading = -heading;
                        heading = wrap(-Pif, Pif, heading - unit.rotation);

                        auto pitch = (Pif / 2.0f) - std::acos(aimVector.dot(Vector3f(0.0f, 1.0f, 0.0f)) / aimVector.length());

                        auto threadId = unit.cobEnvironment->createThread("AimPrimary", {toTaAngle(RadiansAngle(heading)).value, toTaAngle(RadiansAngle(pitch)).value});

                        if (threadId)
                        {
                            weapon.state = UnitWeaponStateAiming{*threadId};
                        }
                        else
                        {
                            // We couldn't launch an aiming script (there isn't one)
                            // FIXME: should transition straight to firing state here
                            weapon.state = UnitWeaponStateIdle();
                        }
                    }
                    else if (auto aimingState = boost::get<UnitWeaponStateAiming>(&weapon.state); aimingState != nullptr)
                    {
                        auto returnValue = unit.cobEnvironment->tryReapThread(aimingState->aimingThread);
                        if (returnValue)
                        {
                            // FIXME: should transition to firing state here
                            weapon.state = UnitWeaponStateIdle();

                            // pretend we killed the target
                            unit.cobEnvironment->createThread("TargetCleared", {0});
                            unit.orders.pop_front();
                        }
                    }
                }
            }
        }

        applyUnitSteering(unitId, targetAngle, targetSpeed);

        if (unit.currentSpeed > 0.0f && previousSpeed == 0.0f)
        {
            unit.cobEnvironment->createThread("StartMoving");
        }
        else if (unit.currentSpeed == 0.0f && previousSpeed > 0.0f)
        {
            unit.cobEnvironment->createThread("StopMoving");
        }

        updateUnitPosition(unitId);
    }

    void UnitBehaviorService::applyUnitSteering(UnitId id, float targetAngle, float targetSpeed)
    {
        updateUnitRotation(id, targetAngle);
        updateUnitSpeed(id, targetSpeed);
    }

    void UnitBehaviorService::updateUnitRotation(UnitId id, float targetAngle)
    {
        auto& unit = scene->getSimulation().getUnit(id);

        auto angleDelta = wrap(-Pif, Pif, targetAngle - unit.rotation);

        auto turnRateThisFrame = unit.turnRate;
        if (std::abs(angleDelta) <= turnRateThisFrame)
        {
            unit.rotation = targetAngle;
        }
        else
        {
            unit.rotation = wrap(-Pif, Pif, unit.rotation + (turnRateThisFrame * (angleDelta > 0.0f ? 1.0f : -1.0f)));
        }
    }

    void UnitBehaviorService::updateUnitSpeed(UnitId id, float targetSpeed)
    {
        auto& unit = scene->getSimulation().getUnit(id);

        if (targetSpeed > unit.currentSpeed)
        {
            // accelerate to target speed
            if (targetSpeed - unit.currentSpeed <= unit.acceleration)
            {
                unit.currentSpeed = targetSpeed;
            }
            else
            {
                unit.currentSpeed += unit.acceleration;
            }
        }
        else
        {
            // brake to target speed
            if (unit.currentSpeed - targetSpeed <= unit.brakeRate)
            {
                unit.currentSpeed = targetSpeed;
            }
            else
            {
                unit.currentSpeed -= unit.brakeRate;
            }
        }

        auto effectiveMaxSpeed = unit.maxSpeed;
        if (unit.position.y < scene->getTerrain().getSeaLevel())
        {
            effectiveMaxSpeed /= 2.0f;
        }
        unit.currentSpeed = std::clamp(unit.currentSpeed, 0.0f, effectiveMaxSpeed);
    }

    void UnitBehaviorService::updateUnitPosition(UnitId unitId)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        auto direction = Matrix4f::rotationY(unit.rotation) * Vector3f(0.0f, 0.0f, -1.0f);

        unit.inCollision = false;

        if (unit.currentSpeed > 0.0f)
        {
            auto newPosition = unit.position + (direction * unit.currentSpeed);
            newPosition.y = scene->getTerrain().getHeightAt(newPosition.x, newPosition.z);

            if (!tryApplyMovementToPosition(unitId, newPosition))
            {
                unit.inCollision = true;

                // if we failed to move, try in each axis separately
                // to see if we can complete a "partial" movement
                const Vector3f maskX(0.0f, 1.0f, 1.0f);
                const Vector3f maskZ(1.0f, 1.0f, 0.0f);

                Vector3f newPos1;
                Vector3f newPos2;
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

    bool UnitBehaviorService::tryApplyMovementToPosition(UnitId id, const Vector3f& newPosition)
    {
        auto& sim = scene->getSimulation();
        auto& unit = sim.getUnit(id);

        // check for collision at the new position
        auto newFootprintRegion = scene->computeFootprintRegion(newPosition, unit.footprintX, unit.footprintZ);

        // Unlike for pathfinding, TA doesn't care about the unit's actual movement class for collision checks,
        // it only cares about the attributes defined directly on the unit.
        // Jam these into an ad-hoc movement class to pass into our walkability check.
        MovementClass mc;
        mc.minWaterDepth = unit.minWaterDepth;
        mc.maxWaterDepth = unit.maxWaterDepth;
        mc.maxSlope = unit.maxSlope;
        mc.maxWaterSlope = unit.maxWaterSlope;
        mc.footprintX = unit.footprintX;
        mc.footprintZ = unit.footprintZ;

        if (!isGridPointWalkable(sim.terrain, mc, newFootprintRegion.x, newFootprintRegion.y))
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
}
