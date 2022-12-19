#include "UnitBehaviorService_util.h"

#include <stdexcept>

namespace rwe
{
    SimAngle angleTo(const Vector2x<SimScalar>& lhs, const Vector2x<SimScalar>& rhs)
    {
        return atan2(lhs.det(rhs), lhs.dot(rhs));
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

    SimScalar getTurnRadius(SimScalar speed, SimScalar turnRate)
    {
        return speed / angularToRadians(turnRate);
    }

    std::optional<SimVector> findLandingLocation(const GameSimulation& sim, ConstUnitInfo unitInfo)
    {
        // TODO: make this smarter
        return unitInfo.state->position;
    }

    std::pair<SimAngle, SimAngle> computeHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to, SimScalar speed, SimScalar gravity, SimScalar zOffset, ProjectilePhysicsType projectileType)
    {
        return match(
            projectileType,
            [&](const ProjectilePhysicsTypeLineOfSight&) {
                return computeLineOfSightHeadingAndPitch(rotation, from, to);
            },
            [&](const ProjectilePhysicsTypeBallistic&) {
                return computeBallisticHeadingAndPitch(rotation, from, to, speed, gravity, zOffset);
            });
    }

    std::pair<SimAngle, SimAngle> computeLineOfSightHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to)
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

    std::pair<SimAngle, SimAngle> computeBallisticHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to, SimScalar speed, SimScalar gravity, SimScalar zOffset)
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

    SteeringInfo arrive(const UnitState& unit, const UnitDefinition& unitDefinition, const UnitPhysicsInfoGround& physics, const SimVector& destination)
    {
        SimVector xzPosition(unit.position.x, 0_ss, unit.position.z);
        SimVector xzDestination(destination.x, 0_ss, destination.z);
        auto distanceSquared = xzPosition.distanceSquared(xzDestination);
        auto brakingDistance = (physics.currentSpeed * physics.currentSpeed) / (2_ss * unitDefinition.brakeRate);

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

    SimScalar computeNewGroundUnitSpeed(const MapTerrain& terrain, const UnitState& unit, const UnitDefinition& unitDefinition, const UnitPhysicsInfoGround& physics)
    {
        SimScalar newSpeed;
        if (physics.steeringInfo.targetSpeed > physics.currentSpeed)
        {
            // accelerate to target speed
            if (physics.steeringInfo.targetSpeed - physics.currentSpeed <= unitDefinition.acceleration)
            {
                newSpeed = physics.steeringInfo.targetSpeed;
            }
            else
            {
                newSpeed = physics.currentSpeed + unitDefinition.acceleration;
            }
        }
        else
        {
            // brake to target speed
            if (physics.currentSpeed - physics.steeringInfo.targetSpeed <= unitDefinition.brakeRate)
            {
                newSpeed = physics.steeringInfo.targetSpeed;
            }
            else
            {
                newSpeed = physics.currentSpeed - unitDefinition.brakeRate;
            }
        }

        auto effectiveMaxSpeed = unitDefinition.maxVelocity;
        if (unit.position.y < terrain.getSeaLevel())
        {
            effectiveMaxSpeed /= 2_ss;
        }
        newSpeed = std::clamp(newSpeed, 0_ss, effectiveMaxSpeed);

        return newSpeed;
    }

    SimVector decelerate(SimVector currentVelocity, SimScalar deceleration)
    {
        auto currentDirection = currentVelocity.normalizedOr(SimVector(0_ss, 0_ss, 0_ss));
        if (currentDirection == SimVector(0_ss, 0_ss, 0_ss))
        {
            return SimVector(0_ss, 0_ss, 0_ss);
        }
        auto newVelocity = currentVelocity - (currentDirection * deceleration);
        return newVelocity;
    }

    SimVector computeNewAirUnitVelocity(const UnitState& unit, const UnitDefinition& unitDefinition, const AirMovementStateFlying& physics)
    {
        if (!physics.targetPosition)
        {
            return decelerate(physics.currentVelocity, unitDefinition.acceleration);
        }

        auto rawDirection = *physics.targetPosition - unit.position;
        auto distanceSquared = rawDirection.lengthSquared();
        auto direction = rawDirection.normalizedOr(SimVector(0_ss, 0_ss, 0_ss));

        auto currentSpeedSquared = physics.currentVelocity.lengthSquared();
        auto decelerationDistance = currentSpeedSquared / (2_ss * unitDefinition.acceleration);

        if (distanceSquared > (decelerationDistance * decelerationDistance))
        {
            auto targetVelocity = direction * unitDefinition.maxVelocity;
            auto velocityDelta = targetVelocity - physics.currentVelocity;
            auto deltaDirection = velocityDelta.normalizedOr(SimVector(0_ss, 0_ss, 0_ss));

            auto newVelocity = physics.currentVelocity + (deltaDirection * unitDefinition.acceleration);
            if (newVelocity.lengthSquared() > (unitDefinition.maxVelocity * unitDefinition.maxVelocity))
            {
                newVelocity = newVelocity.normalized() * unitDefinition.maxVelocity;
            }
            return newVelocity;
        }
        else
        {
            return decelerate(physics.currentVelocity, unitDefinition.acceleration);
        }
    }

    SimVector findClosestPoint(const DiscreteRect& rect, const SimVector& p)
    {
        SimScalar x;
        if (p.x < SimScalar(rect.left()))
        {
            x = SimScalar(rect.left());
        }
        else if (p.x > SimScalar(rect.right()))
        {
            x = SimScalar(rect.right());
        }
        else
        {
            x = p.x;
        }

        SimScalar z;
        if (p.z < SimScalar(rect.top()))
        {
            z = SimScalar(rect.top());
        }
        else if (p.x > SimScalar(rect.bottom()))
        {
            z = SimScalar(rect.bottom());
        }
        else
        {
            z = p.z;
        }

        return SimVector(x, p.y, z);
    }

    bool hasReachedGoal(const UnitState& unit, const NavigationGoal& goal)
    {
        auto destination = match(
            goal,
            [&](const SimVector& pos) {
                return std::make_optional(pos);
            },
            [&](const DiscreteRect& rect) {
                return std::make_optional(findClosestPoint(rect, unit.position));
            },
            [&](const NavigationGoalLandingLocation&) {
                const auto& s = std::get_if<NavigationStateMovingToLandingSpot>(&unit.navigationState.state);
                if (s)
                {
                    return std::make_optional(s->landingLocation);
                }
                return std::optional<SimVector>();
            });

        if (!destination)
        {
            return false;
        }

        SimVector xzPosition(unit.position.x, 0_ss, unit.position.z);
        SimVector xzDestination(destination->x, 0_ss, destination->z);
        auto distanceSquared = xzPosition.distanceSquared(xzDestination);

        if (distanceSquared < (8_ss * 8_ss))
        {
            return true;
        }

        return false;
    }

    std::string getAimScriptName(unsigned int weaponIndex)
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

    std::string getAimFromScriptName(unsigned int weaponIndex)
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

    std::string getFireScriptName(unsigned int weaponIndex)
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

    std::string getQueryScriptName(unsigned int weaponIndex)
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
}
