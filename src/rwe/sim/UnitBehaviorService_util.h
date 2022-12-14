#pragma once

#include <rwe/math/Vector2x.h>
#include <rwe/sim/GameSimulation.h>
#include <rwe/sim/SimAngle.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/sim/SimVector.h>
#include <rwe/sim/UnitDefinition.h>
#include <rwe/sim/UnitState.h>
#include <string>

namespace rwe
{
    SimAngle angleTo(const Vector2x<SimScalar>& lhs, const Vector2x<SimScalar>& rhs);

    SimVector toDirection(SimAngle heading, SimAngle pitch);

    SimVector rotateDirectionXZ(const SimVector& direction, SimAngle angle);

    SimScalar getTurnRadius(SimScalar speed, SimScalar turnRate);

    std::optional<SimVector> findLandingLocation(const GameSimulation& sim, const UnitState& unitState, const UnitDefinition& unitDefinition);

    std::pair<SimAngle, SimAngle> computeHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to, SimScalar speed, SimScalar gravity, SimScalar zOffset, ProjectilePhysicsType projectileType);

    std::pair<SimAngle, SimAngle> computeLineOfSightHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to);

    std::pair<SimAngle, SimAngle> computeBallisticHeadingAndPitch(SimAngle rotation, const SimVector& from, const SimVector& to, SimScalar speed, SimScalar gravity, SimScalar zOffset);

    std::optional<std::pair<SimAngle, SimAngle>> computeFiringAngles(SimScalar speed, SimScalar gravity, SimScalar targetX, SimScalar targetY);

    SteeringInfo seek(const UnitState& unit, const UnitDefinition& unitDefinition, const SimVector& destination);

    SteeringInfo arrive(const UnitState& unit, const UnitDefinition& unitDefinition, const UnitPhysicsInfoGround& physics, const SimVector& destination);

    SimScalar computeNewGroundUnitSpeed(const MapTerrain& terrain, const UnitState& unit, const UnitDefinition& unitDefinition, const UnitPhysicsInfoGround& physics);

    SimVector decelerate(SimVector currentVelocity, SimScalar deceleration);

    SimVector computeNewAirUnitVelocity(const UnitState& unit, const UnitDefinition& unitDefinition, const AirMovementStateFlying& physics);

    bool isFlying(const UnitPhysicsInfo& physics);

    SimVector findClosestPoint(const DiscreteRect& rect, const SimVector& p);

    bool hasReachedGoal(const UnitState& unit, const NavigationGoal& goal);

    std::string getAimScriptName(unsigned int weaponIndex);
    std::string getAimFromScriptName(unsigned int weaponIndex);
    std::string getFireScriptName(unsigned int weaponIndex);
    std::string getQueryScriptName(unsigned int weaponIndex);

}
