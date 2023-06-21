#pragma once

#include <rwe/grid/Grid.h>
#include <rwe/sim/Energy.h>
#include <rwe/sim/Metal.h>
#include <rwe/sim/MovementClassId.h>
#include <rwe/sim/SimScalar.h>
#include <string>
#include <variant>

namespace rwe
{
    enum class YardMapCell
    {
        GroundPassableWhenOpen,
        WaterPassableWhenOpen,
        GroundNoFeature,
        GroundGeoPassableWhenOpen,
        Geo,
        Ground,
        GroundPassableWhenClosed,
        Water,
        GroundPassable,
        WaterPassable,
        Passable
    };

    struct UnitDefinition
    {
        struct NamedMovementClass
        {
            MovementClassId movementClassId;
        };
        struct AdHocMovementClass
        {
            unsigned int footprintX;
            unsigned int footprintZ;
            unsigned int maxSlope;
            unsigned int maxWaterSlope;
            unsigned int minWaterDepth;
            unsigned int maxWaterDepth;
        };
        using MovementCollisionInfo = std::variant<NamedMovementClass, AdHocMovementClass>;

        // FIXME: these two things should probably be in unit media info??
        // They are not needed for sim.
        std::string unitName;
        std::string unitDescription;

        std::string objectName;

        MovementCollisionInfo movementCollisionInfo;

        /**
         * Rate at which the unit turns in world angular units/tick.
         */
        SimScalar turnRate;

        /**
         * Maximum speed the unit can travel forwards in game units/tick.
         */
        SimScalar maxVelocity;

        /**
         * Speed at which the unit accelerates in game units/tick.
         */
        SimScalar acceleration;

        /**
         * Speed at which the unit brakes in game units/tick.
         */
        SimScalar brakeRate;

        bool canAttack;
        bool canMove;
        bool canGuard;

        /** If true, the unit is considered a commander for victory conditions. */
        bool commander;

        unsigned int maxHitPoints;

        bool isMobile;

        bool floater;
        bool canHover;

        bool canFly;

        /** Distance above the ground that the unit flies at. */
        SimScalar cruiseAltitude;

        std::string weapon1;
        std::string weapon2;
        std::string weapon3;

        std::string explodeAs;

        bool builder;
        unsigned int buildTime;
        Energy buildCostEnergy;
        Metal buildCostMetal;

        unsigned int workerTimePerTick;

        SimScalar buildDistance;

        bool onOffable;
        bool activateWhenBuilt;

        Energy energyMake;
        Metal metalMake;
        Energy energyUse;
        Metal metalUse;

        Metal makesMetal;
        Metal extractsMetal;

        Energy energyStorage;
        Metal metalStorage;

        std::optional<Grid<YardMapCell>> yardMap;

        std::string corpse;

        bool hideDamage;
        bool showPlayerName;

        std::string soundCategory;
    };
}
