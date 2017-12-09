#include "UnitBehaviorService.h"
#include <rwe/GameScene.h>
#include <rwe/math/rwe_math.h>

namespace rwe
{
    UnitBehaviorService::UnitBehaviorService(GameScene* scene) : scene(scene)
    {
    }

    void UnitBehaviorService::update(UnitId unitId)
    {
        auto& unit = scene->getSimulation().getUnit(unitId);

        float previousSpeed = unit.currentSpeed;

        // check our orders
        if (!unit.orders.empty())
        {
            const auto& order = unit.orders.front();

            // process move orders
            auto moveOrder = boost::get<MoveOrder>(&order);
            if (moveOrder != nullptr)
            {
                Vector2f xzPosition(unit.position.x, unit.position.z);
                const auto& destination = moveOrder->destination;
                Vector2f xzDestination(destination.x, destination.z);
                auto distanceSquared = xzPosition.distanceSquared(xzDestination);
                if (distanceSquared < 1.0f)
                {
                    // order complete
                    unit.orders.pop_front();
                    if (unit.arrivedSound)
                    {
                        scene->playSoundOnSelectChannel(*unit.arrivedSound);
                    }
                }
                // TODO: if distance <= braking distance, brake
                else
                {
                    auto effectiveMaxSpeed = unit.maxSpeed;
                    if (unit.position.y < scene->getTerrain().getSeaLevel())
                    {
                        effectiveMaxSpeed /= 2.0f;
                    }

                    // accelerate to max speed
                    auto accelerationThisFrame = unit.acceleration;
                    if (effectiveMaxSpeed - unit.currentSpeed <= accelerationThisFrame)
                    {
                        unit.currentSpeed = effectiveMaxSpeed;
                    }
                    else
                    {
                        unit.currentSpeed += accelerationThisFrame;
                    }
                }

                // steer towards the goal
                auto xzDirection = xzDestination - xzPosition;
                auto destAngle = Vector2f(0.0f, -1.0f).angleTo(xzDirection);
                destAngle = (2.0f * Pif) - destAngle; // convert to anticlockwise in our coordinate system
                auto angleDelta = wrap(-Pif, Pif, destAngle - unit.rotation);

                auto turnRateThisFrame = unit.turnRate;
                if (std::abs(angleDelta) <= turnRateThisFrame)
                {
                    unit.rotation = destAngle;
                }
                else
                {
                    unit.rotation = wrap(-Pif, Pif, unit.rotation + (turnRateThisFrame * (angleDelta > 0.0f ? 1.0f : -1.0f)));
                }
            }
        }
        else
        {
            if (unit.currentSpeed < unit.brakeRate)
            {
                unit.currentSpeed = 0.0f;
            }
            else
            {
                unit.currentSpeed -= unit.brakeRate;
            }
        }

        if (unit.currentSpeed > 0.0f && previousSpeed == 0.0f)
        {
            unit.cobEnvironment->createThread("StartMoving");
        }
        else if (unit.currentSpeed == 0.0f && previousSpeed > 0.0f)
        {
            unit.cobEnvironment->createThread("StopMoving");
        }

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
