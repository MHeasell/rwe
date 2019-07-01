#include "GameSimulation.h"
#include "collection_util.h"
#include <rwe/overloaded.h>

#include <rwe/movement.h>

namespace rwe
{
    bool GamePlayerInfo::addResourceDelta(const Energy& apparentEnergy,
                                          const Metal&  apparentMetal,
                                          const Energy& actualEnergy,
                                          const Metal&  actualMetal)
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

    bool PathRequest::operator==(const PathRequest& rhs) const { return unitId == rhs.unitId; }

    bool PathRequest::operator!=(const PathRequest& rhs) const { return !(rhs == *this); }

    GameSimulation::GameSimulation(MapTerrain&& terrain, unsigned char surfaceMetal)
        : terrain(std::move(terrain)),
          occupiedGrid(this->terrain.getHeightMap().getWidth() - 1,
                       this->terrain.getHeightMap().getHeight() - 1,
                       OccupiedCell()),
          metalGrid(
              this->terrain.getHeightMap().getWidth() - 1, this->terrain.getHeightMap().getHeight() - 1, surfaceMetal)
    {
    }

    FeatureId GameSimulation::addFeature(MapFeature&& newFeature)
    {
        auto featureId = nextFeatureId;
        auto pair      = features.insert_or_assign(featureId, std::move(newFeature));
        nextFeatureId  = FeatureId(nextFeatureId.value + 1);


        auto& f = pair.first->second;
        if (f.isBlocking)
        {
            auto footprintRegion = computeFootprintRegion(f.position, f.footprintX, f.footprintZ);
            occupiedGrid.forEach(occupiedGrid.clipRegion(footprintRegion),
                                 [featureId](auto& cell) { cell.occupiedType = OccupiedFeature(featureId); });
        }

        if (!f.isBlocking && f.isIndestructible && f.metal)
        {
            auto footprintRegion = computeFootprintRegion(f.position, f.footprintX, f.footprintZ);
            metalGrid.set(metalGrid.clipRegion(footprintRegion), f.metal);
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
        auto unitId = nextUnitId;

        // set footprint area as occupied by the unit
        auto footprintRect = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        if (isCollisionAt(footprintRect, unitId))
        {
            return std::nullopt;
        }

        auto footprintRegion = occupiedGrid.tryToRegion(footprintRect);
        assert(!!footprintRegion);

        if (unit.isMobile)
        {
            occupiedGrid.forEach(*footprintRegion, [unitId](auto& cell) { cell.occupiedType = OccupiedUnit(unitId); });
        }
        else
        {
            assert(!!unit.yardMap);
            occupiedGrid.forEach2(
                footprintRegion->x, footprintRegion->y, *unit.yardMap, [&](auto& cell, const auto& yardMapCell) {
                    cell.buildingCell = BuildingOccupiedCell{unitId, isPassable(yardMapCell, unit.yardOpen)};
                });
        }

        units.insert_or_assign(unitId, std::move(unit));

        nextUnitId = UnitId(nextUnitId.value + 1);

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

    DiscreteRect GameSimulation::computeFootprintRegion(const Vector3f& position,
                                                        unsigned int    footprintX,
                                                        unsigned int    footprintZ) const
    {
        auto     halfFootprintX = static_cast<float>(footprintX) * MapTerrain::HeightTileWidthInWorldUnits / 2.0f;
        auto     halfFootprintZ = static_cast<float>(footprintZ) * MapTerrain::HeightTileHeightInWorldUnits / 2.0f;
        Vector3f topLeft(position.x - halfFootprintX, position.y, position.z - halfFootprintZ);

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

    bool
    GameSimulation::isYardmapBlocked(unsigned int x, unsigned int y, const Grid<YardMapCell>& yardMap, bool open) const
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
        return isCollisionAt(top) || isCollisionAt(bottom) || isCollisionAt(left) || isCollisionAt(right);
    }

    void GameSimulation::showObject(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).mesh.find(name);
        if (mesh)
        {
            mesh->get().visible = true;
        }
    }

    void GameSimulation::hideObject(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).mesh.find(name);
        if (mesh)
        {
            mesh->get().visible = false;
        }
    }

    void GameSimulation::enableShading(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).mesh.find(name);
        if (mesh)
        {
            mesh->get().shaded = true;
        }
    }

    void GameSimulation::disableShading(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).mesh.find(name);
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

    std::optional<std::reference_wrapper<Unit>> GameSimulation::tryGetUnit(UnitId id) { return tryFind(units, id); }

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

    GamePlayerInfo& GameSimulation::getPlayer(PlayerId player) { return players.at(player.value); }

    const GamePlayerInfo& GameSimulation::getPlayer(PlayerId player) const { return players.at(player.value); }

    void GameSimulation::moveObject(UnitId unitId, const std::string& name, Axis axis, float position, float speed)
    {
        getUnit(unitId).moveObject(name, axis, position, speed);
    }

    void GameSimulation::moveObjectNow(UnitId unitId, const std::string& name, Axis axis, float position)
    {
        getUnit(unitId).moveObjectNow(name, axis, position);
    }

    void GameSimulation::turnObject(UnitId unitId, const std::string& name, Axis axis, RadiansAngle angle, float speed)
    {
        getUnit(unitId).turnObject(name, axis, angle, speed);
    }

    void GameSimulation::turnObjectNow(UnitId unitId, const std::string& name, Axis axis, RadiansAngle angle)
    {
        getUnit(unitId).turnObjectNow(name, axis, angle);
    }

    void GameSimulation::spinObject(UnitId unitId, const std::string& name, Axis axis, float speed, float acceleration)
    {
        getUnit(unitId).spinObject(name, axis, speed, acceleration);
    }

    void GameSimulation::stopSpinObject(UnitId unitId, const std::string& name, Axis axis, float deceleration)
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

    std::optional<UnitId> GameSimulation::getFirstCollidingUnit(const Ray3f& ray) const
    {
        auto                  bestDistance = std::numeric_limits<float>::infinity();
        std::optional<UnitId> it;

        for (const auto& [id, unit] : units)
        {
            auto distance = unit.selectionIntersect(ray);
            if (distance && distance < bestDistance)
            {
                bestDistance = *distance;
                it           = id;
            }
        }

        return it;
    }

    std::optional<UnitId> GameSimulation::getClosestUnit(const Vector3f& position, float epsilon) const
    {
        for (const auto& [id, unit] : units)
        {
            if (unit.position.distance(position) <= epsilon)
            {
                return id;
            }
        }
        return {};
    }

    std::optional<Vector3f> GameSimulation::intersectLineWithTerrain(const Line3f& line) const
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

    LaserProjectile GameSimulation::createProjectileFromWeapon(PlayerId          owner,
                                                               const UnitWeapon& weapon,
                                                               const Vector3f&   position,
                                                               const Vector3f&   direction)
    {
        LaserProjectile laser;
        laser.owner    = owner;
        laser.position = position;
        laser.origin   = position;
        laser.velocity = direction * weapon.velocity;
        laser.duration = weapon.duration;
        laser.color    = weapon.color;
        laser.color2   = weapon.color2;

        laser.endSmoke   = weapon.endSmoke;
        laser.smokeTrail = weapon.smokeTrail;
        laser.lastSmoke  = gameTime;

        laser.soundHit   = weapon.soundHit;
        laser.soundWater = weapon.soundWater;

        laser.explosion      = weapon.explosion;
        laser.waterExplosion = weapon.waterExplosion;

        laser.damage = weapon.damage;

        laser.damageRadius = weapon.damageRadius;

        return laser;
    }

    void GameSimulation::spawnLaser(PlayerId          owner,
                                    const UnitWeapon& weapon,
                                    const Vector3f&   position,
                                    const Vector3f&   direction)
    {
        auto laser = createProjectileFromWeapon(owner, weapon, position, direction);

        auto it = std::find_if(lasers.begin(), lasers.end(), [](const auto& x) { return !x; });
        if (it == lasers.end())
        {
            lasers.push_back(laser);
        }
        else
        {
            *it = laser;
        }
    }

    void GameSimulation::spawnExplosion(const Vector3f& position, const std::shared_ptr<SpriteSeries>& animation)
    {
        Explosion exp;
        exp.position  = position;
        exp.animation = animation;
        exp.startTime = gameTime;

        auto it = std::find_if(explosions.begin(), explosions.end(), [](const auto& x) { return !x; });
        if (it == explosions.end())
        {
            explosions.push_back(exp);
        }
        else
        {
            *it = exp;
        }
    }

    void GameSimulation::spawnSmoke(const Vector3f& position, const std::shared_ptr<SpriteSeries>& animation)
    {
        Explosion exp;
        exp.position  = position;
        exp.animation = animation;
        exp.startTime = gameTime;
        exp.floats    = true;

        auto it = std::find_if(explosions.begin(), explosions.end(), [](const auto& x) { return !x; });
        if (it == explosions.end())
        {
            explosions.push_back(exp);
        }
        else
        {
            *it = exp;
        }
    }

    WinStatus GameSimulation::computeWinStatus() const
    {
        std::optional<PlayerId> livingPlayer;
        for (unsigned int i = 0; i < players.size(); ++i)
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

    bool GameSimulation::addResourceDelta(const UnitId& unitId,
                                          const Energy& apparentEnergy,
                                          const Metal&  apparentMetal,
                                          const Energy& actualEnergy,
                                          const Metal&  actualMetal)
    {
        auto& unit   = getUnit(unitId);
        auto& player = getPlayer(unit.owner);

        unit.addEnergyDelta(apparentEnergy);
        unit.addMetalDelta(apparentMetal);
        return player.addResourceDelta(apparentEnergy, apparentMetal, actualEnergy, actualMetal);
    }

    bool GameSimulation::trySetYardOpen(const UnitId& unitId, bool open)
    {
        auto& unit            = getUnit(unitId);
        auto  footprintRect   = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        auto  footprintRegion = occupiedGrid.tryToRegion(footprintRect);
        assert(!!footprintRegion);

        assert(!!unit.yardMap);
        if (isYardmapBlocked(footprintRegion->x, footprintRegion->y, *unit.yardMap, open))
        {
            return false;
        }

        occupiedGrid.forEach2(
            footprintRegion->x, footprintRegion->y, *unit.yardMap, [&](auto& cell, const auto& yardMapCell) {
                cell.buildingCell = BuildingOccupiedCell{unitId, isPassable(yardMapCell, open)};
            });

        unit.yardOpen = open;

        return true;
    }

    void GameSimulation::emitBuggerOff(const UnitId& unitId)
    {
        auto& unit            = getUnit(unitId);
        auto  footprintRect   = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        auto  footprintRegion = occupiedGrid.tryToRegion(footprintRect);
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
}
