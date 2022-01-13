#include "GameSimulation.h"
#include <rwe/Index.h>
#include <rwe/collection_util.h>
#include <rwe/match.h>
#include <rwe/sim/SimScalar.h>

#include <rwe/GameHash_util.h>
#include <rwe/sim/movement.h>
#include <type_traits>

namespace rwe
{
    bool GamePlayerInfo::addResourceDelta(const Energy& apparentEnergy, const Metal& apparentMetal, const Energy& actualEnergy, const Metal& actualMetal)
    {
        if (recordAndCheckDesire(apparentEnergy) && recordAndCheckDesire(apparentMetal))
        {
            acceptResource(actualEnergy);
            acceptResource(actualMetal);
            return true;
        }

        return false;
    }

    bool GamePlayerInfo::recordAndCheckDesire(const rwe::Energy& energy)
    {
        if (energy >= Energy(0))
        {
            return true;
        }

        desiredEnergyConsumptionBuffer -= energy;
        return !energyStalled;
    }

    bool GamePlayerInfo::recordAndCheckDesire(const rwe::Metal& metal)
    {
        if (metal >= Metal(0))
        {
            return true;
        }

        desiredMetalConsumptionBuffer -= metal;
        return !metalStalled;
    }

    void GamePlayerInfo::acceptResource(const rwe::Energy& energy)
    {
        if (energy >= Energy(0))
        {
            energyProductionBuffer += energy;
        }
        else
        {
            actualEnergyConsumptionBuffer -= energy;
        }
    }

    void GamePlayerInfo::acceptResource(const rwe::Metal& metal)
    {
        if (metal >= Metal(0))
        {
            metalProductionBuffer += metal;
        }
        else
        {
            actualMetalConsumptionBuffer -= metal;
        }
    }

    bool PathRequest::operator==(const PathRequest& rhs) const
    {
        return unitId == rhs.unitId;
    }

    bool PathRequest::operator!=(const PathRequest& rhs) const
    {
        return !(rhs == *this);
    }

    GameSimulation::GameSimulation(MapTerrain&& terrain, unsigned char surfaceMetal)
        : terrain(std::move(terrain)),
          occupiedGrid(this->terrain.getHeightMap().getWidth() - 1, this->terrain.getHeightMap().getHeight() - 1, OccupiedCell()),
          metalGrid(this->terrain.getHeightMap().getWidth() - 1, this->terrain.getHeightMap().getHeight() - 1, surfaceMetal)
    {
    }

    // FIXME: the signature of this is really awkward,
    // caller shouldn't have to supply feature definition.
    // One day we should fix this so that the sim knows all the definitions.
    FeatureId GameSimulation::addFeature(const FeatureDefinition& featureDefinition, MapFeature&& newFeature)
    {
        auto featureId = FeatureId(features.emplace(std::move(newFeature)));

        auto& f = features.tryGet(featureId)->get();
        if (featureDefinition.blocking)
        {
            auto footprintRegion = computeFootprintRegion(f.position, featureDefinition.footprintX, featureDefinition.footprintZ);
            occupiedGrid.forEach(occupiedGrid.clipRegion(footprintRegion), [featureId](auto& cell) { cell.occupiedType = OccupiedFeature(featureId); });
        }

        if (!featureDefinition.blocking && featureDefinition.indestructible && featureDefinition.metal)
        {
            auto footprintRegion = computeFootprintRegion(f.position, featureDefinition.footprintX, featureDefinition.footprintZ);
            metalGrid.set(metalGrid.clipRegion(footprintRegion), featureDefinition.metal);
        }

        return featureId;
    }

    PlayerId GameSimulation::addPlayer(const GamePlayerInfo& info)
    {
        PlayerId id(players.size());
        players.push_back(info);
        return id;
    }

    std::optional<UnitId> GameSimulation::tryAddUnit(Unit&& unit)
    {
        // set footprint area as occupied by the unit
        auto footprintRect = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        if (isCollisionAt(footprintRect))
        {
            return std::nullopt;
        }

        auto unitId = units.emplace(std::move(unit));
        const auto& insertedUnit = units.tryGet(unitId)->get();

        auto footprintRegion = occupiedGrid.tryToRegion(footprintRect);
        assert(!!footprintRegion);

        if (insertedUnit.isMobile)
        {
            occupiedGrid.forEach(*footprintRegion, [unitId](auto& cell) { cell.occupiedType = OccupiedUnit(unitId); });
        }
        else
        {
            assert(!!insertedUnit.yardMap);
            occupiedGrid.forEach2(footprintRegion->x, footprintRegion->y, *insertedUnit.yardMap, [&](auto& cell, const auto& yardMapCell) {
                cell.buildingCell = BuildingOccupiedCell{unitId, isPassable(yardMapCell, insertedUnit.yardOpen)};
            });
        }

        return unitId;
    }

    bool GameSimulation::canBeBuiltAt(const rwe::MovementClass& mc, unsigned int x, unsigned int y) const
    {
        if (isCollisionAt(DiscreteRect(x, y, mc.footprintX, mc.footprintZ)))
        {
            return false;
        }

        if (!isGridPointWalkable(terrain, mc, x, y))
        {
            return false;
        }

        return true;
    }

    DiscreteRect GameSimulation::computeFootprintRegion(const SimVector& position, unsigned int footprintX, unsigned int footprintZ) const
    {
        auto halfFootprintX = SimScalar(footprintX * MapTerrain::HeightTileWidthInWorldUnits.value / 2);
        auto halfFootprintZ = SimScalar(footprintZ * MapTerrain::HeightTileHeightInWorldUnits.value / 2);
        SimVector topLeft(
            position.x - halfFootprintX,
            position.y,
            position.z - halfFootprintZ);

        auto cell = terrain.worldToHeightmapCoordinateNearest(topLeft);

        return DiscreteRect(cell.x, cell.y, footprintX, footprintZ);
    }

    bool GameSimulation::isCollisionAt(const DiscreteRect& rect) const
    {
        auto region = occupiedGrid.tryToRegion(rect);
        if (!region)
        {
            return true;
        }

        return isCollisionAt(*region);
    }

    bool GameSimulation::isCollisionAt(const GridRegion& region) const
    {
        return occupiedGrid.any(region, [&](const auto& cell) {
            auto isColliding = match(
                cell.occupiedType,
                [](const OccupiedNone&) { return false; },
                [](const OccupiedUnit&) { return true; },
                [](const OccupiedFeature&) { return true; });
            if (isColliding)
            {
                return true;
            }

            if (cell.buildingCell && !cell.buildingCell->passable)
            {
                return true;
            }

            return false;
        });
    }

    bool GameSimulation::isCollisionAt(const DiscreteRect& rect, UnitId self) const
    {
        auto region = occupiedGrid.tryToRegion(rect);
        if (!region)
        {
            return true;
        }

        return occupiedGrid.any(*region, [&](const auto& cell) {
            auto inCollision = match(
                cell.occupiedType,
                [&](const OccupiedNone&) { return false; },
                [&](const OccupiedUnit& u) { return u.id != self; },
                [&](const OccupiedFeature&) { return true; });
            if (inCollision)
            {
                return true;
            }

            if (cell.buildingCell && cell.buildingCell->unit != self && !cell.buildingCell->passable)
            {
                return true;
            }

            return false;
        });
    }

    bool GameSimulation::isYardmapBlocked(unsigned int x, unsigned int y, const Grid<YardMapCell>& yardMap, bool open) const
    {
        return occupiedGrid.any2(x, y, yardMap, [&](const auto& cell, const auto& yardMapCell) {
            if (isPassable(yardMapCell, open))
            {
                return false;
            }

            auto inCollision = match(
                cell.occupiedType,
                [&](const OccupiedNone&) { return false; },
                [&](const OccupiedUnit&) { return true; },
                [&](const OccupiedFeature&) { return true; });
            if (inCollision)
            {
                return true;
            }

            return false;
        });
    }

    bool GameSimulation::isAdjacentToObstacle(const DiscreteRect& rect) const
    {
        DiscreteRect top(rect.x - 1, rect.y - 1, rect.width + 2, 1);
        DiscreteRect bottom(rect.x - 1, rect.y + rect.width, rect.width + 2, 1);
        DiscreteRect left(rect.x - 1, rect.y, 1, rect.height);
        DiscreteRect right(rect.x + rect.width, rect.y, 1, rect.height);
        return isCollisionAt(top)
            || isCollisionAt(bottom)
            || isCollisionAt(left)
            || isCollisionAt(right);
    }

    void GameSimulation::showObject(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).findPiece(name);
        if (mesh)
        {
            mesh->get().visible = true;
        }
    }

    void GameSimulation::hideObject(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).findPiece(name);
        if (mesh)
        {
            mesh->get().visible = false;
        }
    }

    void GameSimulation::enableShading(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).findPiece(name);
        if (mesh)
        {
            mesh->get().shaded = true;
        }
    }

    void GameSimulation::disableShading(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).findPiece(name);
        if (mesh)
        {
            mesh->get().shaded = false;
        }
    }

    Unit& GameSimulation::getUnit(UnitId id)
    {
        auto it = units.find(id);
        assert(it != units.end());
        return it->second;
    }

    const Unit& GameSimulation::getUnit(UnitId id) const
    {
        auto it = units.find(id);
        assert(it != units.end());
        return it->second;
    }

    std::optional<std::reference_wrapper<Unit>> GameSimulation::tryGetUnit(UnitId id)
    {
        return tryFind(units, id);
    }

    std::optional<std::reference_wrapper<const Unit>> GameSimulation::tryGetUnit(UnitId id) const
    {
        return tryFind(units, id);
    }

    bool GameSimulation::unitExists(UnitId id) const
    {
        auto it = units.find(id);
        return it != units.end();
    }

    MapFeature& GameSimulation::getFeature(FeatureId id)
    {
        auto it = features.find(id);
        assert(it != features.end());
        return it->second;
    }

    const MapFeature& GameSimulation::getFeature(FeatureId id) const
    {
        auto it = features.find(id);
        assert(it != features.end());
        return it->second;
    }

    GamePlayerInfo& GameSimulation::getPlayer(PlayerId player)
    {
        return players.at(player.value);
    }

    const GamePlayerInfo& GameSimulation::getPlayer(PlayerId player) const
    {
        return players.at(player.value);
    }

    void GameSimulation::moveObject(UnitId unitId, const std::string& name, Axis axis, SimScalar position, SimScalar speed)
    {
        getUnit(unitId).moveObject(name, axis, position, speed);
    }

    void GameSimulation::moveObjectNow(UnitId unitId, const std::string& name, Axis axis, SimScalar position)
    {
        getUnit(unitId).moveObjectNow(name, axis, position);
    }

    void GameSimulation::turnObject(UnitId unitId, const std::string& name, Axis axis, SimAngle angle, SimScalar speed)
    {
        getUnit(unitId).turnObject(name, axis, angle, speed);
    }

    void GameSimulation::turnObjectNow(UnitId unitId, const std::string& name, Axis axis, SimAngle angle)
    {
        getUnit(unitId).turnObjectNow(name, axis, angle);
    }

    void GameSimulation::spinObject(UnitId unitId, const std::string& name, Axis axis, SimScalar speed, SimScalar acceleration)
    {
        getUnit(unitId).spinObject(name, axis, speed, acceleration);
    }

    void GameSimulation::stopSpinObject(UnitId unitId, const std::string& name, Axis axis, SimScalar deceleration)
    {
        getUnit(unitId).stopSpinObject(name, axis, deceleration);
    }

    bool GameSimulation::isPieceMoving(UnitId unitId, const std::string& name, Axis axis) const
    {
        return getUnit(unitId).isMoveInProgress(name, axis);
    }

    bool GameSimulation::isPieceTurning(UnitId unitId, const std::string& name, Axis axis) const
    {
        return getUnit(unitId).isTurnInProgress(name, axis);
    }

    std::optional<SimVector> GameSimulation::intersectLineWithTerrain(const Line3x<SimScalar>& line) const
    {
        return terrain.intersectLine(line);
    }

    void GameSimulation::moveUnitOccupiedArea(const DiscreteRect& oldRect, const DiscreteRect& newRect, UnitId unitId)
    {
        auto oldRegion = occupiedGrid.tryToRegion(oldRect);
        assert(!!oldRegion);
        auto newRegion = occupiedGrid.tryToRegion(newRect);
        assert(!!newRegion);

        occupiedGrid.forEach(*oldRegion, [](auto& cell) { cell.occupiedType = OccupiedNone(); });
        occupiedGrid.forEach(*newRegion, [unitId](auto& cell) { cell.occupiedType = OccupiedUnit(unitId); });
    }

    void GameSimulation::requestPath(UnitId unitId)
    {
        PathRequest request{unitId};

        // If the unit is already in the queue for a path,
        // we'll assume that they no longer care about their old request
        // and that their new request is for some new path,
        // so we'll move them to the back of the queue for fairness.
        auto it = std::find(pathRequests.begin(), pathRequests.end(), request);
        if (it != pathRequests.end())
        {
            pathRequests.erase(it);
        }

        pathRequests.push_back(PathRequest{unitId});
    }

    Projectile GameSimulation::createProjectileFromWeapon(
        PlayerId owner, const UnitWeapon& weapon, const SimVector& position, const SimVector& direction, SimScalar distanceToTarget)
    {
        const auto& weaponDefinition = weaponDefinitions.at(weapon.weaponType);

        Projectile projectile;
        projectile.weaponType = weapon.weaponType;
        projectile.owner = owner;
        projectile.position = position;
        projectile.previousPosition = position;
        projectile.origin = position;
        projectile.velocity = direction * weaponDefinition.velocity;
        projectile.gravity = weaponDefinition.physicsType == ProjectilePhysicsType::Ballistic;

        projectile.lastSmoke = gameTime;

        projectile.damage = weaponDefinition.damage;

        projectile.damageRadius = weaponDefinition.damageRadius;

        if (weaponDefinition.weaponTimer)
        {
            auto randomDecay = weaponDefinition.randomDecay.value().value;
            std::uniform_int_distribution<unsigned int> dist(0, randomDecay);
            auto randomVal = dist(rng);
            projectile.dieOnFrame = gameTime + *weaponDefinition.weaponTimer - GameTime(randomDecay / 2) + GameTime(randomVal);
        }
        else if (weaponDefinition.physicsType == ProjectilePhysicsType::LineOfSight)
        {
            projectile.dieOnFrame = gameTime + GameTime(simScalarToUInt(distanceToTarget / weaponDefinition.velocity) + 1);
        }

        projectile.createdAt = gameTime;
        projectile.groundBounce = weaponDefinition.groundBounce;

        return projectile;
    }

    void GameSimulation::spawnProjectile(PlayerId owner, const UnitWeapon& weapon, const SimVector& position, const SimVector& direction, SimScalar distanceToTarget)
    {
        projectiles.emplace(createProjectileFromWeapon(owner, weapon, position, direction, distanceToTarget));
    }

    WinStatus GameSimulation::computeWinStatus() const
    {
        std::optional<PlayerId> livingPlayer;
        for (Index i = 0; i < getSize(players); ++i)
        {
            const auto& p = players[i];

            if (p.status == GamePlayerStatus::Alive)
            {
                if (livingPlayer)
                {
                    // multiple players are alive, the game is not over
                    return WinStatusUndecided();
                }
                else
                {
                    livingPlayer = PlayerId(i);
                }
            }
        }

        if (livingPlayer)
        {
            // one player is alive, declare them the winner
            return WinStatusWon{*livingPlayer};
        }

        // no players are alive, the game is a draw
        return WinStatusDraw();
    }

    bool GameSimulation::addResourceDelta(const UnitId& unitId, const Energy& energy, const Metal& metal)
    {
        return addResourceDelta(unitId, energy, metal, energy, metal);
    }

    bool GameSimulation::addResourceDelta(const UnitId& unitId, const Energy& apparentEnergy, const Metal& apparentMetal, const Energy& actualEnergy, const Metal& actualMetal)
    {
        auto& unit = getUnit(unitId);
        auto& player = getPlayer(unit.owner);

        unit.addEnergyDelta(apparentEnergy);
        unit.addMetalDelta(apparentMetal);
        return player.addResourceDelta(apparentEnergy, apparentMetal, actualEnergy, actualMetal);
    }

    bool GameSimulation::trySetYardOpen(const UnitId& unitId, bool open)
    {
        auto& unit = getUnit(unitId);
        auto footprintRect = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        auto footprintRegion = occupiedGrid.tryToRegion(footprintRect);
        assert(!!footprintRegion);

        assert(!!unit.yardMap);
        if (isYardmapBlocked(footprintRegion->x, footprintRegion->y, *unit.yardMap, open))
        {
            return false;
        }

        occupiedGrid.forEach2(footprintRegion->x, footprintRegion->y, *unit.yardMap, [&](auto& cell, const auto& yardMapCell) {
            cell.buildingCell = BuildingOccupiedCell{unitId, isPassable(yardMapCell, open)};
        });

        unit.yardOpen = open;

        return true;
    }

    void GameSimulation::emitBuggerOff(const UnitId& unitId)
    {
        auto& unit = getUnit(unitId);
        auto footprintRect = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        auto footprintRegion = occupiedGrid.tryToRegion(footprintRect);
        assert(!!footprintRegion);

        occupiedGrid.forEach(*footprintRegion, [&](const auto& e) {
            auto unitId = match(
                e.occupiedType,
                [&](const OccupiedUnit& u) { return std::optional(u.id); },
                [&](const auto&) { return std::optional<UnitId>(); });

            if (unitId)
            {
                tellToBuggerOff(*unitId, footprintRect);
            }
        });
    }

    void GameSimulation::tellToBuggerOff(const UnitId& unitId, const DiscreteRect& rect)
    {
        auto& unit = getUnit(unitId);
        if (unit.orders.empty())
        {
            unit.addOrder(BuggerOffOrder(rect));
        }
    }

    GameHash GameSimulation::computeHash() const
    {
        return computeHashOf(*this);
    }
}
