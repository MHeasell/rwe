#include "GameSimulation.h"

namespace rwe
{
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

    void GameSimulation::addFeature(MapFeature&& newFeature)
    {
        auto featureId = features.size();
        features.push_back(std::move(newFeature));

        auto& f = features[featureId];
        if (f.isBlocking)
        {
            auto footprintRegion = computeFootprintRegion(f.position, f.footprintX, f.footprintZ);
            occupiedGrid.grid.setArea(occupiedGrid.grid.clipRegion(footprintRegion), OccupiedFeature());
        }
    }

    PlayerId GameSimulation::addPlayer(const GamePlayerInfo& info)
    {
        PlayerId id(players.size());
        players.push_back(info);
        return id;
    }

    UnitId GameSimulation::predictNextUnitId() const
    {
        return UnitId(units.size());
    }

    bool GameSimulation::tryAddUnit(Unit&& unit)
    {
        auto unitId = predictNextUnitId();

        // set footprint area as occupied by the unit
        auto footprintRect = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        if (isCollisionAt(footprintRect, unitId))
        {
            return false;
        }

        auto footprintRegion = occupiedGrid.grid.tryToRegion(footprintRect);
        assert(!!footprintRegion);

        occupiedGrid.grid.setArea(*footprintRegion, OccupiedUnit(unitId));

        units.push_back(std::move(unit));

        return true;
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

    void GameSimulation::showObject(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).mesh.find(name);
        if (mesh)
        {
            mesh->visible = true;
        }
    }

    void GameSimulation::hideObject(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).mesh.find(name);
        if (mesh)
        {
            mesh->visible = false;
        }
    }

    Unit& GameSimulation::getUnit(UnitId id)
    {
        return units.at(id.value);
    }

    const Unit& GameSimulation::getUnit(UnitId id) const
    {
        return units.at(id.value);
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

    void GameSimulation::turnObject(UnitId unitId, const std::string& name, Axis axis, float angle, float speed)
    {
        getUnit(unitId).turnObject(name, axis, angle, speed);
    }

    void GameSimulation::turnObjectNow(UnitId unitId, const std::string& name, Axis axis, float angle)
    {
        getUnit(unitId).turnObjectNow(name, axis, angle);
    }

    bool GameSimulation::isPieceMoving(UnitId unitId, const std::string& name, Axis axis) const
    {
        return getUnit(unitId).isMoveInProgress(name, axis);
    }

    bool GameSimulation::isPieceTurning(UnitId unitId, const std::string& name, Axis axis) const
    {
        return getUnit(unitId).isTurnInProgress(name, axis);
    }

    boost::optional<UnitId> GameSimulation::getFirstCollidingUnit(const Ray3f& ray) const
    {
        auto bestDistance = std::numeric_limits<float>::infinity();
        boost::optional<UnitId> it;

        for (unsigned int i = 0; i < units.size(); ++i)
        {
            auto distance = units[i].selectionIntersect(ray);
            if (distance && distance < bestDistance)
            {
                bestDistance = *distance;
                it = UnitId(i);
            }
        }

        return it;
    }

    boost::optional<Vector3f> GameSimulation::intersectLineWithTerrain(const Line3f& line) const
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
}
