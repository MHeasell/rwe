#include "GameSimulation.h"

namespace rwe
{
    bool PathRequest::operator==(const PathRequest& rhs) const
    {
        return unitId == rhs.unitId;
    }

    bool PathRequest::operator!=(const PathRequest& rhs) const
    {
        return !(rhs == *this);
    }

    class IsCollisionVisitor : public boost::static_visitor<bool>
    {
    private:
        UnitId unitId;

    public:
        explicit IsCollisionVisitor(const UnitId& unitId) : unitId(unitId)
        {
        }

        bool operator()(const OccupiedNone&) const
        {
            return false;
        }
        bool operator()(const OccupiedUnit& u) const
        {
            return u.id != unitId;
        }
        bool operator()(const OccupiedFeature&) const
        {
            return true;
        }
    };

    GameSimulation::GameSimulation(MapTerrain&& terrain)
        : terrain(std::move(terrain)),
          occupiedGrid(this->terrain.getHeightMap().getWidth(), this->terrain.getHeightMap().getHeight())
    {
    }

    FeatureId GameSimulation::addFeature(MapFeature&& newFeature)
    {
        auto featureId = nextFeatureId;
        auto pair = features.insert_or_assign(featureId, std::move(newFeature));
        nextFeatureId = FeatureId(nextFeatureId.value + 1);


        auto& f = pair.first->second;
        if (f.isBlocking)
        {
            auto footprintRegion = computeFootprintRegion(f.position, f.footprintX, f.footprintZ);
            occupiedGrid.grid.setArea(occupiedGrid.grid.clipRegion(footprintRegion), OccupiedFeature(featureId));
        }

        return featureId;
    }

    PlayerId GameSimulation::addPlayer(const GamePlayerInfo& info)
    {
        PlayerId id(players.size());
        players.push_back(info);
        return id;
    }

    bool GameSimulation::tryAddUnit(Unit&& unit)
    {
        auto unitId = nextUnitId;

        // set footprint area as occupied by the unit
        auto footprintRect = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        if (isCollisionAt(footprintRect, unitId))
        {
            return false;
        }

        auto footprintRegion = occupiedGrid.grid.tryToRegion(footprintRect);
        assert(!!footprintRegion);

        occupiedGrid.grid.setArea(*footprintRegion, OccupiedUnit(unitId));

        units.insert_or_assign(unitId, std::move(unit));

        nextUnitId = UnitId(nextUnitId.value + 1);

        return true;
    }

    bool GameSimulation::deleteUnit(UnitId unit)
    {
        units.erase(unit);
    }

    DiscreteRect GameSimulation::computeFootprintRegion(const Vector3f& position, unsigned int footprintX, unsigned int footprintZ) const
    {
        auto halfFootprintX = static_cast<float>(footprintX) * MapTerrain::HeightTileWidthInWorldUnits / 2.0f;
        auto halfFootprintZ = static_cast<float>(footprintZ) * MapTerrain::HeightTileHeightInWorldUnits / 2.0f;
        Vector3f topLeft(
            position.x - halfFootprintX,
            position.y,
            position.z - halfFootprintZ);

        auto cell = terrain.worldToHeightmapCoordinateNearest(topLeft);

        return DiscreteRect(cell.x, cell.y, footprintX, footprintZ);
    }

    bool GameSimulation::isCollisionAt(const DiscreteRect& rect, UnitId self) const
    {
        auto region = occupiedGrid.grid.tryToRegion(rect);
        if (!region)
        {
            return true;
        }

        for (unsigned int dy = 0; dy < region->height; ++dy)
        {
            for (unsigned int dx = 0; dx < region->width; ++dx)
            {
                const auto& cell = occupiedGrid.grid.get(region->x + dx, region->y + dy);
                if (boost::apply_visitor(IsCollisionVisitor(self), cell))
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool GameSimulation::isAdjacentToObstacle(const DiscreteRect& rect, UnitId self) const
    {
        DiscreteRect expandedRect(rect.x - 1, rect.y - 1, rect.width + 2, rect.height + 2);
        return isCollisionAt(expandedRect, self);
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

    const GamePlayerInfo& GameSimulation::getPlayer(PlayerId player) const
    {
        return players.at(player.value);
    }

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
        auto bestDistance = std::numeric_limits<float>::infinity();
        std::optional<UnitId> it;

        for (const auto& entry : units)
        {
            auto distance = entry.second.selectionIntersect(ray);
            if (distance && distance < bestDistance)
            {
                bestDistance = *distance;
                it = entry.first;
            }
        }

        return it;
    }

    std::optional<Vector3f> GameSimulation::intersectLineWithTerrain(const Line3f& line) const
    {
        return terrain.intersectLine(line);
    }

    void GameSimulation::moveUnitOccupiedArea(const DiscreteRect& oldRect, const DiscreteRect& newRect, UnitId unitId)
    {
        auto oldRegion = occupiedGrid.grid.tryToRegion(oldRect);
        assert(!!oldRegion);
        auto newRegion = occupiedGrid.grid.tryToRegion(newRect);
        assert(!!newRegion);

        occupiedGrid.grid.setArea(*oldRegion, OccupiedNone());
        occupiedGrid.grid.setArea(*newRegion, OccupiedUnit(unitId));
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

    void GameSimulation::spawnLaser(PlayerId owner, const UnitWeapon& weapon, const Vector3f& position, const Vector3f& direction)
    {
        LaserProjectile laser;
        laser.owner = owner;
        laser.position = position;
        laser.origin = position;
        laser.velocity = direction * weapon.velocity;
        laser.duration = weapon.duration;
        laser.color = weapon.color;
        laser.color2 = weapon.color2;

        laser.soundHit = weapon.soundHit;
        laser.soundWater = weapon.soundWater;

        laser.explosion = weapon.explosion;
        laser.waterExplosion = weapon.waterExplosion;

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
        exp.position = position;
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
}
