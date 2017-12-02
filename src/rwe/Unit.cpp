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

    void Unit::turnObject(const std::string& pieceName, Axis axis, float targetAngle, float speed)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        UnitMesh::TurnOperation op(toRadians(targetAngle), toRadians(speed));

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

    void Unit::turnObjectNow(const std::string& pieceName, Axis axis, float targetAngle)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                piece->rotation.x = toRadians(targetAngle);
                piece->xTurnOperation = boost::none;
                break;
            case Axis::Y:
                piece->rotation.y = toRadians(targetAngle);
                piece->yTurnOperation = boost::none;
                break;
            case Axis::Z:
                piece->rotation.z = toRadians(targetAngle);
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
    }

    void Unit::addOrder(const UnitOrder& order)
    {
        orders.push_back(order);
    }

    void Unit::update(GameScene& scene, UnitId id, float dt)
    {
        float previousSpeed = currentSpeed;

        // check our orders
        if (!orders.empty())
        {
            const auto& order = orders.front();

            // process move orders
            auto moveOrder = boost::get<MoveOrder>(&order);
            if (moveOrder != nullptr)
            {
                Vector2f xzPosition(position.x, position.z);
                const auto& destination = moveOrder->destination;
                Vector2f xzDestination(destination.x, destination.z);
                auto distanceSquared = xzPosition.distanceSquared(xzDestination);
                if (distanceSquared < 1.0f)
                {
                    // order complete
                    orders.pop_front();
                    if (arrivedSound)
                    {
                        scene.playSoundOnSelectChannel(*arrivedSound);
                    }
                }
                // TODO: if distance <= braking distance, brake
                else
                {
                    auto effectiveMaxSpeed = maxSpeed;
                    if (position.y < scene.getTerrain().getSeaLevel())
                    {
                        effectiveMaxSpeed /= 2.0f;
                    }

                    // accelerate to max speed
                    auto accelerationThisFrame = acceleration;
                    if (effectiveMaxSpeed - currentSpeed <= accelerationThisFrame)
                    {
                        currentSpeed = effectiveMaxSpeed;
                    }
                    else
                    {
                        currentSpeed += accelerationThisFrame;
                    }
                }

                // steer towards the goal
                auto xzDirection = xzDestination - xzPosition;
                auto destAngle = Vector2f(0.0f, -1.0f).angleTo(xzDirection);
                destAngle = (2.0f * Pif) - destAngle; // convert to anticlockwise in our coordinate system
                auto angleDelta = wrap(-Pif, Pif, destAngle - rotation);

                auto turnRateThisFrame = turnRate;
                if (std::abs(angleDelta) <= turnRateThisFrame)
                {
                    rotation = destAngle;
                }
                else
                {
                    rotation = wrap(-Pif, Pif, rotation + (turnRateThisFrame * (angleDelta > 0.0f ? 1.0f : -1.0f)));
                }
            }
        }
        else
        {
            if (currentSpeed < brakeRate)
            {
                currentSpeed = 0.0f;
            }
            else
            {
                currentSpeed -= brakeRate;
            }
        }

        if (currentSpeed > 0.0f && previousSpeed == 0.0f)
        {
            cobEnvironment->createThread("StartMoving");
        }
        else if (currentSpeed == 0.0f && previousSpeed > 0.0f)
        {
            cobEnvironment->createThread("StopMoving");
        }

        auto direction = Matrix4f::rotationY(rotation) * Vector3f(0.0f, 0.0f, -1.0f);

        if (currentSpeed > 0.0f)
        {
            auto newPosition = position + (direction * currentSpeed);
            newPosition.y = scene.getTerrain().getHeightAt(newPosition.x, newPosition.z);

            if (!tryApplyMovementToPosition(scene, id, newPosition))
            {
                // if we failed to move, try in each axis separately
                // to see if we can complete a "partial" movement
                const Vector3f maskX(0.0f, 1.0f, 1.0f);
                const Vector3f maskZ(1.0f, 1.0f, 0.0f);

                Vector3f newPos1;
                Vector3f newPos2;
                if (direction.x > direction.z)
                {
                    newPos1 = position + (direction * maskZ * currentSpeed);
                    newPos2 = position + (direction * maskX * currentSpeed);
                }
                else
                {
                    newPos1 = position + (direction * maskX * currentSpeed);
                    newPos2 = position + (direction * maskZ * currentSpeed);
                }
                newPos1.y = scene.getTerrain().getHeightAt(newPos1.x, newPos1.z);
                newPos2.y = scene.getTerrain().getHeightAt(newPos2.x, newPos2.z);

                if (!tryApplyMovementToPosition(scene, id, newPos1))
                {
                    tryApplyMovementToPosition(scene, id, newPos2);
                }
            }
        }
    }

    bool Unit::tryApplyMovementToPosition(GameScene& scene, UnitId id, const Vector3f& newPosition)
    {
        // check for collision at the new position and update accordingly
        auto newFootprintRegion = scene.computeFootprintRegion(newPosition, footprintX, footprintZ);
        if (!scene.isCollisionAt(newFootprintRegion, id))
        {
            auto footprintRegion = scene.computeFootprintRegion(position, footprintX, footprintZ);
            scene.moveUnitOccupiedArea(footprintRegion, newFootprintRegion, id);
            position = newPosition;
            return true;
        }

        return false;
    }
}
