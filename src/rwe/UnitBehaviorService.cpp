#include "UnitBehaviorService.h"
#include <rwe/GameScene.h>
#include <rwe/cob/CobExecutionContext.h>
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

        // Clear steering targets.
        unit.targetAngle = unit.rotation;
        unit.targetSpeed = 0.0f;

        // check our orders
        if (!unit.orders.empty())
        {
            const auto& order = unit.orders.front();

            // process move orders
            if (auto moveOrder = boost::get<MoveOrder>(&order); moveOrder != nullptr)
            {
                if (auto idleState = boost::get<IdleState>(&unit.behaviourState); idleState != nullptr)
                {
                    // request a path to follow
                    scene->getSimulation().requestPath(unitId);
                    const auto& destination = moveOrder->destination;
                    unit.behaviourState = MovingState{destination, boost::none, true};
                }
                else if (auto movingState = boost::get<MovingState>(&unit.behaviourState); movingState != nullptr)
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
                        if (followPath(unit, *pathToFollow))
                        {
                            // we finished following the path,
                            // order complete
                            unit.orders.pop_front();
                            unit.behaviourState = IdleState();

                            if (unit.arrivedSound)
                            {
                                scene->playSoundOnSelectChannel(*unit.arrivedSound);
                            }
                        }
                    }
                }
            }
            else if (auto attackGroundOrder = boost::get<AttackGroundOrder>(&order); attackGroundOrder != nullptr)
            {
                if (unit.weapons.empty())
                {
                    unit.orders.pop_front();
                }
                else
                {
                    auto maxRangeSquared = unit.weapons[0].maxRange * unit.weapons[0].maxRange;
                    if (auto idleState = boost::get<IdleState>(&unit.behaviourState); idleState != nullptr)
                    {
                        // if we're out of range, drive into range
                        if (unit.position.distanceSquared(attackGroundOrder->target) > maxRangeSquared)
                        {
                            // request a path to follow
                            scene->getSimulation().requestPath(unitId);
                            const auto& destination = attackGroundOrder->target;
                            unit.behaviourState = MovingState{destination, boost::none, true};
                        }
                        else
                        {
                            // we're in range, aim weapons
                            for (unsigned int i = 0; i < unit.weapons.size(); ++i)
                            {
                                unit.setWeaponTarget(i, attackGroundOrder->target);
                            }
                        }
                    }
                    else if (auto movingState = boost::get<MovingState>(&unit.behaviourState); movingState != nullptr)
                    {
                        if (unit.position.distanceSquared(attackGroundOrder->target) <= maxRangeSquared)
                        {
                            unit.behaviourState = IdleState();
                        }
                        else
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
            }
            else if (auto attackOrder = boost::get<AttackOrder>(&order); attackOrder != nullptr)
            {
                if (unit.weapons.empty())
                {
                    unit.orders.pop_front();
                }
                else
                {
                    // FIXME: this is unsafe, the target unit could already be dead
                    const auto& targetUnit = scene->getSimulation().getUnit(attackOrder->target);
                    const auto& targetPosition = targetUnit.position;

                    auto maxRangeSquared = unit.weapons[0].maxRange * unit.weapons[0].maxRange;
                    if (auto idleState = boost::get<IdleState>(&unit.behaviourState); idleState != nullptr)
                    {
                        // if we're out of range, drive into range
                        if (unit.position.distanceSquared(targetPosition) > maxRangeSquared)
                        {
                            // request a path to follow
                            scene->getSimulation().requestPath(unitId);
                            auto destination = scene->computeFootprintRegion(targetUnit.position, targetUnit.footprintX, targetUnit.footprintZ);
                            unit.behaviourState = MovingState{destination, boost::none, true};
                        }
                        else
                        {
                            // we're in range, aim weapons
                            for (unsigned int i = 0; i < unit.weapons.size(); ++i)
                            {
                                unit.setWeaponTarget(i, targetPosition);
                            }
                        }
                    }
                    else if (auto movingState = boost::get<MovingState>(&unit.behaviourState); movingState != nullptr)
                    {
                        if (unit.position.distanceSquared(targetPosition) <= maxRangeSquared)
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
            }
        }

        for (unsigned int i = 0; i < unit.weapons.size(); ++i)
        {
            updateWeapon(unitId, i);
        }

        applyUnitSteering(unitId);

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

    std::pair<float, float> UnitBehaviorService::computeHeadingAndPitch(float rotation, const Vector3f& from, const Vector3f& to)
    {
        auto aimVector = to - from;
        auto heading = Vector2f(0.0f, -1.0f).angleTo(Vector2f(aimVector.x, aimVector.z));
        heading = -heading;
        heading = wrap(-Pif, Pif, heading - rotation);

        auto pitch = (Pif / 2.0f) - std::acos(aimVector.dot(Vector3f(0.0f, 1.0f, 0.0f)) / aimVector.length());

        return {heading, pitch};
    }

    bool UnitBehaviorService::followPath(Unit& unit, PathFollowingInfo& path)
    {
        const auto& destination = *path.currentWaypoint;
        Vector2f xzPosition(unit.position.x, unit.position.z);
        Vector2f xzDestination(destination.x, destination.z);
        auto distanceSquared = xzPosition.distanceSquared(xzDestination);

        auto isFinalDestination = path.currentWaypoint == (path.path.waypoints.end() - 1);

        if (distanceSquared < (8.0f * 8.0f))
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
            auto destAngle = Vector2f(0.0f, -1.0f).angleTo(xzDirection);
            unit.targetAngle = (2.0f * Pif) - destAngle; // convert to anticlockwise in our coordinate system

            // drive at full speed until we need to brake
            // to turn or to arrive at the goal
            auto brakingDistance = (unit.currentSpeed * unit.currentSpeed) / (2.0f * unit.brakeRate);

            if (isWithinTurningCircle(xzDirection, unit.currentSpeed, unit.turnRate, unit.rotation))
            {
                unit.targetSpeed = 0.0f;
            }
            else if (isFinalDestination && distanceSquared <= (brakingDistance * brakingDistance))
            {
                unit.targetSpeed = 0.0f;
            }
            else
            {
                unit.targetSpeed = unit.maxSpeed;
            }
        }

        return false;
    }

    class GetTargetPosVisitor : public boost::static_visitor<Vector3f>
    {
    private:
        const GameSimulation* sim;

    public:
        explicit GetTargetPosVisitor(const GameSimulation* sim) : sim(sim) {}
        Vector3f operator()(const Vector3f& pos) const { return pos; }
        // FIXME: unit could no longer exist (e.g. it died), currently this is UB.
        Vector3f operator()(UnitId id) const { return sim->getUnit(id).position; }
    };

    void UnitBehaviorService::updateWeapon(UnitId id, unsigned int weaponIndex)
    {
        auto& unit = scene->getSimulation().getUnit(id);
        auto& weapon = unit.weapons[weaponIndex];

        // FIXME: all this logic really needs to come out.
        // Weapons should run their own independent AI logic
        // (that runs every frame regardless of the unit's order).
        // Right now it's possible for a weapon to get stuck waiting for
        // some non-existent aim thread, because orders could change underneath us,
        // and there's loads of other oddities like not calling TargetCleared reliably.

        if (auto idleState = boost::get<UnitWeaponStateIdle>(&weapon.state); idleState != nullptr)
        {
            // TODO: attempt to acquire a target
        }
        else if (auto aimingState = boost::get<UnitWeaponStateAttacking>(&weapon.state); aimingState != nullptr)
        {
            if (!aimingState->aimInfo)
            {
                Vector3f targetPosition = boost::apply_visitor(GetTargetPosVisitor(&scene->getSimulation()), aimingState->target);
                auto aimFromPosition = getAimingPoint(id, weaponIndex);

                auto headingAndPitch = computeHeadingAndPitch(unit.rotation, aimFromPosition, targetPosition);
                auto heading = headingAndPitch.first;
                auto pitch = headingAndPitch.second;

                auto threadId = unit.cobEnvironment->createThread(getAimScriptName(weaponIndex), {toTaAngle(RadiansAngle(heading)).value, toTaAngle(RadiansAngle(pitch)).value});

                if (threadId)
                {
                    aimingState->aimInfo = UnitWeaponStateAttacking::AimInfo{*threadId, heading, pitch};
                }
                else
                {
                    // We couldn't launch an aiming script (there isn't one),
                    // just go straight to firing.
                    tryFireWeapon(id, weaponIndex, targetPosition);
                }
            }
            else
            {
                const auto aimInfo = *aimingState->aimInfo;
                auto returnValue = unit.cobEnvironment->tryReapThread(aimInfo.thread);
                if (returnValue)
                {
                    // we successfully reaped, clear the thread.
                    aimingState->aimInfo = boost::none;

                    if (*returnValue)
                    {
                        // aiming was successful, check the target again for drift
                        Vector3f targetPosition = boost::apply_visitor(GetTargetPosVisitor(&scene->getSimulation()), aimingState->target);
                        auto aimFromPosition = getAimingPoint(id, weaponIndex);

                        auto headingAndPitch = computeHeadingAndPitch(unit.rotation, aimFromPosition, targetPosition);
                        auto heading = headingAndPitch.first;
                        auto pitch = headingAndPitch.second;

                        // if the target is close enough, try to fire
                        if (std::abs(heading - aimInfo.lastHeading) <= weapon.tolerance && std::abs(pitch - aimInfo.lastPitch) <= weapon.pitchTolerance)
                        {
                            tryFireWeapon(id, weaponIndex, targetPosition);
                        }
                    }
                }
            }
        }
    }

    void UnitBehaviorService::tryFireWeapon(UnitId id, unsigned int weaponIndex, const Vector3f& targetPosition)
    {
        auto& unit = scene->getSimulation().getUnit(id);
        auto& weapon = unit.weapons[weaponIndex];

        // wait for the weapon to reload
        auto gameTime = scene->getGameTime();
        if (gameTime < weapon.readyTime)
        {
            return;
        }

        // spawn a projectile from the firing point
        auto firingPoint = getFiringPoint(id, weaponIndex);
        auto targetVector = targetPosition - firingPoint;
        auto projectileVelocity = targetVector.normalized() * (400.0f / 60.0f);
        scene->getSimulation().spawnLaser(firingPoint, projectileVelocity, 4.0f);

        if (weapon.soundStart)
        {
            scene->playUnitSound(id, *weapon.soundStart);
        }
        unit.cobEnvironment->createThread(getFireScriptName(weaponIndex));

        // we are reloading now
        weapon.readyTime = gameTime + deltaSecondsToTicks(weapon.reloadTime);
    }

    void UnitBehaviorService::applyUnitSteering(UnitId id)
    {
        updateUnitRotation(id);
        updateUnitSpeed(id);
    }

    void UnitBehaviorService::updateUnitRotation(UnitId id)
    {
        auto& unit = scene->getSimulation().getUnit(id);

        auto angleDelta = wrap(-Pif, Pif, unit.targetAngle - unit.rotation);

        auto turnRateThisFrame = unit.turnRate;
        if (std::abs(angleDelta) <= turnRateThisFrame)
        {
            unit.rotation = unit.targetAngle;
        }
        else
        {
            unit.rotation = wrap(-Pif, Pif, unit.rotation + (turnRateThisFrame * (angleDelta > 0.0f ? 1.0f : -1.0f)));
        }
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

    boost::optional<int> UnitBehaviorService::runCobQuery(UnitId id, std::string& name)
    {
        auto& unit = scene->getSimulation().getUnit(id);
        auto thread = unit.cobEnvironment->createNonScheduledThread(name, {0});
        if (!thread)
        {
            return boost::none;
        }
        CobExecutionContext context(&scene->getSimulation(), unit.cobEnvironment.get(), &*thread, id);
        auto status = context.execute();
        if (boost::get<CobEnvironment::FinishedStatus>(&status) == nullptr)
        {
            throw std::runtime_error("Synchronous cob query thread blocked before completion");
        }

        auto result = thread->returnLocals[0];
        return result;
    }

    Vector3f UnitBehaviorService::getAimingPoint(UnitId id, unsigned int weaponIndex)
    {
        auto scriptName = getAimFromScriptName(weaponIndex);
        auto pieceId = runCobQuery(id, scriptName);
        if (!pieceId)
        {
            return scene->getSimulation().getUnit(id).position;
        }

        return getPiecePosition(id, *pieceId);
    }

    Vector3f UnitBehaviorService::getFiringPoint(UnitId id, unsigned int weaponIndex)
    {

        auto scriptName = getQueryScriptName(weaponIndex);
        auto pieceId = runCobQuery(id, scriptName);
        if (!pieceId)
        {
            return scene->getSimulation().getUnit(id).position;
        }

        return getPiecePosition(id, *pieceId);
    }

    Vector3f UnitBehaviorService::getPiecePosition(UnitId id, unsigned int pieceId)
    {
        auto& unit = scene->getSimulation().getUnit(id);

        const auto& pieceName = unit.cobEnvironment->_script->pieces.at(pieceId);
        auto pieceTransform = unit.mesh.getPieceTransform(pieceName);
        if (!pieceTransform)
        {
            throw std::logic_error("Failed to find piece offset");
        }

        return unit.getTransform() * (*pieceTransform) * Vector3f(0.0f, 0.0f, 0.0f);
    }
}
