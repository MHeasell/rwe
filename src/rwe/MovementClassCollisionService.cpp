#include "MovementClassCollisionService.h"

namespace rwe
{
    MovementClassId
    MovementClassCollisionService::registerMovementClass(const std::string& className, Grid<char>&& walkableGrid)
    {
        MovementClassId id(nextId++);
        walkableGrids.insert({id, std::move(walkableGrid)});
        movementClassNameMap.insert({className, id});
        return id;
    }

    boost::optional<MovementClassId> MovementClassCollisionService::resolveMovementClass(const std::string& name)
    {
        auto it = movementClassNameMap.find(name);
        if (it == movementClassNameMap.end())
        {
            return boost::none;
        }

        return it->second;
    }

    bool MovementClassCollisionService::isWalkable(MovementClassId movementClass, const Point& position) const
    {
        if (position.x < 0 || position.y < 0)
        {
            return false;
        }

        auto x = static_cast<unsigned int>(position.x);
        auto y = static_cast<unsigned int>(position.y);

        const auto& map = walkableGrids.find(movementClass);
        if (map == walkableGrids.end())
        {
            throw std::logic_error("Grid for movement class not found");
        }

        const auto& grid = map->second;
        if (x >= grid.getWidth() || y >= grid.getHeight())
        {
            return false;
        }

        return grid.get(x, y);
    }

    Grid<char> computeWalkableGrid(const GameSimulation& sim, const MovementClass& movementClass)
    {
        const auto& terrain = sim.terrain;

        const auto width = terrain.getHeightMap().getWidth();
        const auto height = terrain.getHeightMap().getHeight();

        Grid<char> walkableGrid(width, height, false);

        const auto footprintX = movementClass.footprintX;
        const auto footprintY = movementClass.footprintZ;

        for (unsigned int y = 0; y < height - footprintY - 1; ++y)
        {
            for (unsigned int x = 0; x < width - footprintX - 1; ++x)
            {
                walkableGrid.set(x, y, isGridPointWalkable(terrain, movementClass, x, y));
            }
        }

        return walkableGrid;
    }

    bool
    isGridPointWalkable(const MapTerrain& terrain, const MovementClass& movementClass, unsigned int x, unsigned int y)
    {
        if (isMaxSlopeGreaterThan(terrain.getHeightMap(), x, y, movementClass.footprintX, movementClass.footprintZ, movementClass.maxSlope))
        {
            return false;
        }

        if (!isWaterDepthWithinBounds(terrain.getHeightMap(), terrain.getSeaLevel(), x, y, movementClass.footprintX, movementClass.footprintZ, movementClass.minWaterDepth, movementClass.maxWaterDepth))
        {
            return false;
        }

        return true;
    }

    bool isMaxSlopeGreaterThan(const Grid<unsigned char>& heights, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int maxSlope)
    {
        unsigned int minHeight = heights.get(x, y);
        unsigned int maxHeight = heights.get(x, y);

        for (unsigned int dy = 0; dy < height; ++dy)
        {
            for (unsigned int dx = 0; dx < width; ++dx)
            {
                auto cellHeight = heights.get(x + dx, y + dy);
                if (cellHeight < minHeight)
                {
                    minHeight = cellHeight;
                    auto difference = maxHeight - minHeight;
                    if (difference > maxSlope)
                    {
                        return true;
                    }
                }

                if (cellHeight > maxHeight)
                {
                    maxHeight = cellHeight;
                    auto difference = maxHeight - minHeight;
                    if (difference > maxSlope)
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    bool
    isWaterDepthWithinBounds(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int minWaterDepth, unsigned int maxWaterDepth)
    {
        unsigned int minFoundWaterDepth = getWaterDepth(heights, waterLevel, x, y);
        unsigned int maxFoundWaterDepth = getWaterDepth(heights, waterLevel, x, y);

        for (unsigned int dy = 0; dy < height; ++dy)
        {
            for (unsigned int dx = 0; dx < width; ++dx)
            {
                auto cellWaterDepth = getWaterDepth(heights, waterLevel, x + dx, y + dy);
                if (cellWaterDepth < minFoundWaterDepth)
                {
                    minFoundWaterDepth = cellWaterDepth;
                    if (minFoundWaterDepth < minWaterDepth)
                    {
                        return false;
                    }
                }

                if (cellWaterDepth > maxFoundWaterDepth)
                {
                    maxFoundWaterDepth = cellWaterDepth;
                    if (maxFoundWaterDepth > maxWaterDepth)
                    {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    unsigned int
    getWaterDepth(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y)
    {
        auto height = heights.get(x, y);
        return height < waterLevel ? waterLevel - height : 0;
    }
}
