#ifndef RWE_GAMESIMULATION_H
#define RWE_GAMESIMULATION_H

#include "MapFeature.h"
#include "MapTerrain.h"
#include "OccupiedGrid.h"
#include "Unit.h"

namespace rwe
{
    struct GamePlayerInfo
    {
        unsigned int color;
    };

    struct GameSimulation
    {
        MapTerrain terrain;

        OccupiedGrid occupiedGrid;

        std::vector<GamePlayerInfo> players;

        std::vector<MapFeature> features;

        std::vector<Unit> units;

        unsigned int gameTime{0};

        explicit GameSimulation(MapTerrain&& terrain);

        void addFeature(MapFeature&& newFeature);

        PlayerId addPlayer(const GamePlayerInfo& info);

        /**
         * Returns true if the unit was really added, false otherwise.
         * A unit might not be added because it violates collision constraints.
         */
        bool tryAddUnit(Unit&& unit);

        DiscreteRect computeFootprintRegion(const Vector3f& position, unsigned int footprintX, unsigned int footprintZ) const;

        bool isCollisionAt(const DiscreteRect& rect, UnitId self) const;

        bool isAdjacentToObstacle(const DiscreteRect& rect, UnitId self) const;

        void showObject(UnitId unitId, const std::string& name);

        void hideObject(UnitId unitId, const std::string& name);

        Unit& getUnit(UnitId id);

        const Unit& getUnit(UnitId id) const;

        const GamePlayerInfo& getPlayer(PlayerId player) const;

        void moveObject(UnitId unitId, const std::string& name, Axis axis, float position, float speed);

        void moveObjectNow(UnitId unitId, const std::string& name, Axis axis, float position);

        void turnObject(UnitId unitId, const std::string& name, Axis axis, float angle, float speed);

        void turnObjectNow(UnitId unitId, const std::string& name, Axis axis, float angle);

        bool isPieceMoving(UnitId unitId, const std::string& name, Axis axis) const;

        bool isPieceTurning(UnitId unitId, const std::string& name, Axis axis) const;

        boost::optional<UnitId> getFirstCollidingUnit(const Ray3f& ray) const;

        boost::optional<Vector3f> intersectLineWithTerrain(const Line3f& line) const;

        void moveUnitOccupiedArea(const DiscreteRect& oldRect, const DiscreteRect& newRect, UnitId unitId);
    };
}

#endif
