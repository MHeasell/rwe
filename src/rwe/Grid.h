#ifndef RWE_GRID_H
#define RWE_GRID_H

#include <cassert>
#include <functional>
#include <optional>
#include <rwe/DiscreteRect.h>
#include <stdexcept>
#include <vector>

namespace rwe
{
    struct GridCoordinates
    {
        std::size_t x;
        std::size_t y;
        GridCoordinates() = default;
        GridCoordinates(size_t x, size_t y) : x(x), y(y)
        {
        }

        bool operator==(const GridCoordinates& rhs) const
        {
            return x == rhs.x && y == rhs.y;
        }

        bool operator!=(const GridCoordinates& rhs) const
        {
            return !(rhs == *this);
        }
    };

    struct GridRegion
    {
        unsigned int x;
        unsigned int y;
        unsigned int width;
        unsigned int height;

        GridRegion() = default;
        GridRegion(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
            : x(x), y(y), width(width), height(height) {}
    };

    template <typename T>
    class Grid
    {
    private:
        std::size_t width;
        std::size_t height;
        std::vector<T> data;

    public:
        Grid() : width(0), height(0) {}

        Grid(std::size_t width, std::size_t height)
            : width(width), height(height), data(width * height) {}

        Grid(std::size_t width, std::size_t height, const T& initialValue)
            : width(width), height(height), data(width * height, initialValue) {}

        Grid(std::size_t width, std::size_t height, std::vector<T>&& data)
            : width(width), height(height), data(std::move(data))
        {
            assert(this->data.size() == width * height);
        }

        T& get(std::size_t x, std::size_t y)
        {
            return data[toIndex(x, y)];
        }

        const T& get(std::size_t x, std::size_t y) const
        {
            return data[toIndex(x, y)];
        }

        void set(std::size_t x, std::size_t y, const T& value)
        {
            data[toIndex(x, y)] = value;
        }

        void set(std::size_t x, std::size_t y, T&& value)
        {
            data[toIndex(x, y)] = std::move(value);
        }

        bool operator==(const Grid& rhs) const
        {
            return width == rhs.width && height == rhs.height && data == rhs.data;
        }

        bool operator!=(const Grid& rhs) const
        {
            return !(rhs == *this);
        }

        std::size_t toIndex(std::size_t x, std::size_t y) const
        {
            assert(x < width && y < height);
            return (y * width) + x;
        }

        std::size_t getWidth() const
        {
            return width;
        }

        std::size_t getHeight() const
        {
            return height;
        }

        const T* getData() const
        {
            return data.data();
        }

        T* getData()
        {
            return data.data();
        }

        const std::vector<T> getVector() const
        {
            return data;
        }

        std::vector<T> getVector()
        {
            return data;
        }

        void replaceArea(std::size_t x, std::size_t y, const Grid<T>& replacement)
        {
            if (x + replacement.getWidth() > getWidth() || y + replacement.getHeight() > getHeight())
            {
                throw std::logic_error("replacement goes out of bounds");
            }

            for (std::size_t dy = 0; dy < replacement.getHeight(); ++dy)
            {
                for (std::size_t dx = 0; dx < replacement.getWidth(); ++dx)
                {
                    set(x + dx, y + dy, replacement.get(dx, dy));
                }
            }
        }

        void setArea(std::size_t x, std::size_t y, std::size_t width, std::size_t height, const T& value)
        {
            assert(x + width <= this->width);
            assert(y + height <= this->height);

            for (std::size_t dy = 0; dy < height; ++dy)
            {
                for (std::size_t dx = 0; dx < width; ++dx)
                {
                    set(x + dx, y + dy, value);
                }
            }
        }

        void setArea(const GridRegion& region, const T& value)
        {
            setArea(region.x, region.y, region.width, region.height, value);
        }

        template <typename Func>
        bool anyInArea(const GridRegion& region, Func f) const
        {
            assert(region.x + region.width <= this->width);
            assert(region.y + region.height <= this->height);

            for (std::size_t dy = 0; dy < region.height; ++dy)
            {
                for (std::size_t dx = 0; dx < region.width; ++dx)
                {
                    if (f(get(region.x + dx, region.y + dy)))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        template <typename Func>
        void forInArea(const GridRegion& region, Func f)
        {
            assert(region.x + region.width <= this->width);
            assert(region.y + region.height <= this->height);

            for (std::size_t dy = 0; dy < region.height; ++dy)
            {
                for (std::size_t dx = 0; dx < region.width; ++dx)
                {
                    f(get(region.x + dx, region.y + dy));
                }
            }
        }

        template <typename U, typename BinaryFunc>
        void mergeIn(unsigned int x, unsigned int y, const Grid<U>& g, BinaryFunc f)
        {
            assert(x + g.getWidth() <= this->width);
            assert(y + g.getHeight() <= this->height);

            for (std::size_t dy = 0; dy < g.getHeight(); ++dy)
            {
                for (std::size_t dx = 0; dx < g.getWidth(); ++dx)
                {
                    f(get(x + dx, y + dy), g.get(dx, dy));
                }
            }
        }

        template <typename U, typename BinaryFunc>
        bool anyInArea2(unsigned int x, unsigned int y, const Grid<U>& g, BinaryFunc f) const
        {
            assert(x + g.getWidth() <= this->width);
            assert(y + g.getHeight() <= this->height);

            for (std::size_t dy = 0; dy < g.getHeight(); ++dy)
            {
                for (std::size_t dx = 0; dx < g.getWidth(); ++dx)
                {
                    if (f(get(x + dx, y + dy), g.get(dx, dy)))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        template <typename U, typename BinaryFunc>
        U accumulateArea(const GridRegion& region, U initialValue, BinaryFunc f) const
        {
            assert(region.x + region.width <= this->width);
            assert(region.y + region.height <= this->height);

            for (std::size_t dy = 0; dy < region.height; ++dy)
            {
                for (std::size_t dx = 0; dx < region.width; ++dx)
                {
                    initialValue = f(initialValue, get(region.x + dx, region.y + dy));
                }
            }

            return initialValue;
        }

        /**
         * Returns true if the given rect is completely contained by the grid.
         */
        bool contains(const DiscreteRect& rect) const
        {
            return rect.x >= 0
                && rect.y >= 0
                && static_cast<unsigned int>(rect.x) + rect.width <= width
                && static_cast<unsigned int>(rect.y) + rect.height <= height;
        }

        /**
         * Converts the given discrete rect into a grid region
         * as long as it is contained entirely within the grid.
         */
        std::optional<GridRegion> tryToRegion(const DiscreteRect& rect) const
        {
            if (!contains(rect))
            {
                return std::nullopt;
            }

            return GridRegion(rect.x, rect.y, rect.width, rect.height);
        }

        /**
         * Converts the given point to grid coordinates.
         * If the point lies outside of the grid,
         * the point is clamped to the edge of the grid
         * before being returned.
         *
         * If the grid is of width 0 or height 0,
         * there can be no coordinate that is inside the grid
         * and the return value of this method is undefined.
         */
        GridCoordinates clampToCoords(const Point& p) const
        {
            std::size_t x = (p.x > 0) ? static_cast<std::size_t>(p.x) : 0;
            if (x >= width)
            {
                x = width - 1;
            }

            std::size_t y = (p.y > 0) ? static_cast<std::size_t>(p.y) : 0;
            if (y >= height)
            {
                y = height - 1;
            }

            return GridCoordinates(x, y);
        }

        /**
         * If the given point is within the bounds of the grid
         * returns the value at the corresponding grid cell.
         */
        std::optional<std::reference_wrapper<const T>> tryGet(const Point& p) const
        {
            if (p.x < 0 || p.y < 0)
            {
                return std::nullopt;
            }

            auto x1 = static_cast<std::size_t>(p.x);
            auto y1 = static_cast<std::size_t>(p.y);

            if (x1 >= width || y1 >= height)
            {
                return std::nullopt;
            }

            return get(x1, y1);
        }

        template <typename U>
        void transformAndReplaceArea(std::size_t x, std::size_t y, const Grid<U>& replacement, const std::function<T(const U&)>& transformation)
        {
            if (x + replacement.getWidth() > getWidth() || y + replacement.getHeight() > getHeight())
            {
                throw std::logic_error("replacement goes out of bounds");
            }

            for (std::size_t dy = 0; dy < replacement.getHeight(); ++dy)
            {
                for (std::size_t dx = 0; dx < replacement.getWidth(); ++dx)
                {
                    set(x + dx, y + dy, transformation(replacement.get(dx, dy)));
                }
            }
        }

        /**
         * Returns a clipped version of region
         * representing the intersection of region
         * with the grid bounds.
         * That is, the returned region is will be the portion of the input
         * that lies inside the grid.
         */
        GridRegion clipRegion(const DiscreteRect& rect) const
        {
            auto intersect = rect.intersection(DiscreteRect(0, 0, width, height));
            if (!intersect)
            {
                return GridRegion(0, 0, 0, 0);
            }
            return GridRegion(
                static_cast<unsigned int>(intersect->x), // guaranteed non-negative
                static_cast<unsigned int>(intersect->y), // guaranteed non-negative
                intersect->width,
                intersect->height);
        }
    };
}

#endif
