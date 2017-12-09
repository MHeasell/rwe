#include "UnitBehaviorService.h"
#include <rwe/GameScene.h>
#include <rwe/math/rwe_math.h>

namespace rwe
{
    UnitBehaviorService::UnitBehaviorService(GameScene* scene, PathFindingService* pathFindingService)
        : scene(scene), pathFindingService(pathFindingService)
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
            auto moveOrder = boost::get<MoveOrder>(&order);
            if (moveOrder != nullptr)
            {
                // update path status info
                if (!unit.pathStatus)
                {
                    const auto& destination = moveOrder->destination;
                    auto pathRequest = pathFindingService->getPath(unitId, destination);
                    unit.pathStatus = PathStatusRequested{std::move(pathRequest)};
                }
                else
                {
                    auto pathRequest = boost::get<PathStatusRequested>(&*unit.pathStatus);
                    if (pathRequest != nullptr)
                    {
                        auto status = pathRequest->token.result.wait_for(std::chrono::milliseconds::zero());
                        if (status == std::future_status::ready)
                        {
                            unit.pathStatus = PathStatusFollowing(pathRequest->token.result.get());
                        }
                    }
                }

                auto pathToFollow = boost::get<PathStatusFollowing>(&*unit.pathStatus);

                // if a path is available, attempt to follow it
                if (pathToFollow != nullptr)
                {
                    const auto& destination = *pathToFollow->currentWaypoint;
                    Vector2f xzPosition(unit.position.x, unit.position.z);
                    Vector2f xzDestination(destination.x, destination.z);
                    auto distanceSquared = xzPosition.distanceSquared(xzDestination);

                    auto isFinalDestination = pathToFollow->currentWaypoint == (pathToFollow->path.waypoints.end() - 1);

                    if (distanceSquared < 1.0f)
                    {
                        if (isFinalDestination)
                        {
                            // order complete
                            unit.orders.pop_front();
                            unit.pathStatus = boost::none;

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
                        // to arrive at the goal
                        auto brakingDistance = (unit.currentSpeed * unit.currentSpeed) / (2.0f * unit.brakeRate);
                        if (isFinalDestination && distanceSquared <= (brakingDistance * brakingDistance))
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

        if (unit.currentSpeed > 0.0f)
        {
            auto newPosition = unit.position + (direction * unit.currentSpeed);
            newPosition.y = scene->getTerrain().getHeightAt(newPosition.x, newPosition.z);

            if (!tryApplyMovementToPosition(unitId, newPosition))
            {
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
        auto& unit = scene->getSimulation().getUnit(id);

        // check for collision at the new position and update accordingly
        auto newFootprintRegion = scene->computeFootprintRegion(newPosition, unit.footprintX, unit.footprintZ);
        if (!scene->isCollisionAt(newFootprintRegion, id))
        {
            auto footprintRegion = scene->computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
            scene->moveUnitOccupiedArea(footprintRegion, newFootprintRegion, id);
            unit.position = newPosition;
            return true;
        }

        return false;
    }
}
