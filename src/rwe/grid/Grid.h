#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <optional>
#include <rwe/grid/DiscreteRect.h>
#include <stdexcept>
#include <vector>

namespace rwe
{
    struct GridCoordinates
    {
        int x;
        int y;
        GridCoordinates() = default;
        GridCoordinates(int x, int y) : x(x), y(y)
        {
            assert(x >= 0 && y >= 0);
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
        static GridRegion fromCoordinates(GridCoordinates p1, GridCoordinates p2)
        {
            auto x = std::minmax(p1.x, p2.x);
            auto y = std::minmax(p1.y, p2.y);
            return GridRegion(x.first, y.first, x.second - x.first + 1, y.second - y.first + 1);
        }

        int x;
        int y;
        int width;
        int height;

        GridRegion() = default;

        GridRegion(int x, int y, int width, int height)
            : x(x), y(y), width(width), height(height) 
        { 
            assert(x >= 0 && y >= 0 && width >= 0 && height >= 0); 
        }

        template <typename Func>
        void forEach(Func f) const
        {
            for (int dy = 0; dy < height; ++dy)
            {
                for (int dx = 0; dx < width; ++dx)
                {
                    f(GridCoordinates(x + dx, y + dy));
                }
            }
        }

        template <typename Func>
        void forEach(Func f)
        {
            for (int dy = 0; dy < height; ++dy)
            {
                for (int dx = 0; dx < width; ++dx)
                {
                    f(GridCoordinates(x + dx, y + dy));
                }
            }
        }

        template <typename Func>
        bool any(Func f) const
        {
            for (int dy = 0; dy < height; ++dy)
            {
                for (int dx = 0; dx < width; ++dx)
                {
                    if (f(GridCoordinates(x + dx, y + dy)))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        template <typename U, typename BinaryFunc>
        U accumulate(U initialValue, BinaryFunc f) const
        {
            forEach([&](const auto& c) { initialValue = f(initialValue, c); });
            return initialValue;
        }
    };

    template <typename T>
    class Grid
    {
    private:
        int width;
        int height;
        std::vector<T> data;

    public:
        template <typename Func>
        static Grid<T> from(int width, int height, Func f)
        {
            Grid<T> g(width, height);
            g.fill(f);
            return g;
        }

        Grid() : width(0), height(0) {}

        Grid(int width, int height)
            : width(width), height(height), data(width * height) { assert(width >= 0 && height >= 0); }

        Grid(int width, int height, const T& initialValue)
            : width(width), height(height), data(width * height, initialValue) { assert(width >= 0 && height >= 0); }

        Grid(int width, int height, std::vector<T>&& data)
            : width(width), height(height), data(std::move(data))
        {
            assert(width >= 0 && height >= 0);
            assert(this->data.size() == width * height);
        }

        T& get(int x, int y)
        {
            return data[toIndex(x, y)];
        }

        const T& get(int x, int y) const
        {
            return data[toIndex(x, y)];
        }

        T& get(const GridCoordinates& coords)
        {
            return data[toIndex(coords)];
        }

        const T& get(const GridCoordinates& coords) const
        {
            return data[toIndex(coords)];
        }

        void set(int x, int y, const T& value)
        {
            data[toIndex(x, y)] = value;
        }

        void set(int x, int y, T&& value)
        {
            data[toIndex(x, y)] = std::move(value);
        }

        void set(const GridCoordinates& coords, const T& value)
        {
            data[toIndex(coords)] = value;
        }

        void set(const GridCoordinates& coords, T&& value)
        {
            data[toIndex(coords)] = std::move(value);
        }

        void set(const GridRegion& region, const T& value)
        {
            assert(region.x + region.width <= width);
            assert(region.y + region.height <= height);
            region.forEach([&](const auto& c) { set(c, value); });
        }

        bool operator==(const Grid& rhs) const
        {
            return width == rhs.width && height == rhs.height && data == rhs.data;
        }

        bool operator!=(const Grid& rhs) const
        {
            return !(rhs == *this);
        }

        std::size_t toIndex(int x, int y) const
        {
            assert(x < width && y < height);
            return (static_cast<std::size_t>(y) * width) + x;
        }

        std::size_t toIndex(const GridCoordinates& coords) const
        {
            assert(coords.x < width && coords.y < height);
            return (static_cast<std::size_t>(coords.y) * width) + coords.x;
        }

        int getWidth() const
        {
            return width;
        }

        int getHeight() const
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

        GridRegion getRegion() const
        {
            return GridRegion(0, 0, width, height);
        }

        void replace(int x, int y, const Grid<T>& replacement)
        {
            if (x + replacement.getWidth() > getWidth() || y + replacement.getHeight() > getHeight())
            {
                throw std::logic_error("replacement goes out of bounds");
            }

            for (int dy = 0; dy < replacement.getHeight(); ++dy)
            {
                for (int dx = 0; dx < replacement.getWidth(); ++dx)
                {
                    set(x + dx, y + dy, replacement.get(dx, dy));
                }
            }
        }

        template <typename Func>
        bool any(Func f) const
        {
            return getRegion().any([&](const auto& c) { return f(get(c)); });
        }

        template <typename Func>
        bool any(const GridRegion& region, Func f) const
        {
            assert(region.x + region.width <= width);
            assert(region.y + region.height <= height);
            return region.any([&](const auto& c) { return f(get(c)); });
        }

        template <typename Func>
        bool anyIndexed(Func f) const
        {
            return getRegion().any([&](const auto& c) { return f(c, get(c)); });
        }

        template <typename U, typename BinaryFunc>
        bool any2(int x, int y, const Grid<U>& g, BinaryFunc f) const
        {
            assert(x + g.getWidth() <= this->width);
            assert(y + g.getHeight() <= this->height);
            return g.anyIndexed([&](const auto& c, const auto& v) { return f(get(GridCoordinates(x + c.x, y + c.y)), v); });
        }

        template <typename Func>
        void forEach(const GridRegion& region, Func f)
        {
            assert(region.x + region.width <= width);
            assert(region.y + region.height <= height);
            region.forEach([&](const auto& c) { f(get(c)); });
        }

        template <typename U, typename BinaryFunc>
        void forEach2(int x, int y, const Grid<U>& g, BinaryFunc f)
        {
            assert(x + g.getWidth() <= this->width);
            assert(y + g.getHeight() <= this->height);
            g.forEachIndexed([&](const auto& c, const auto& v) { f(get(GridCoordinates(x + c.x, y + c.y)), v); });
        }

        template <typename Func>
        void forEachIndexed(Func f) const
        {
            getRegion().forEach([&](const auto& c) { f(c, get(c)); });
        }

        template <typename Func>
        void forEachIndexed(Func f)
        {
            getRegion().forEach([&](const auto& c) { f(c, get(c)); });
        }

        template <typename U, typename BinaryFunc>
        U accumulate(const GridRegion& region, U initialValue, BinaryFunc f) const
        {
            assert(region.x + region.width <= this->width);
            assert(region.y + region.height <= this->height);
            return region.accumulate(
                initialValue,
                [&](const auto& v, const auto& c) { return f(v, get(c)); });
        }

        template <typename Func>
        void fill(Func f)
        {
            getRegion().forEach([&](const auto& c) { set(c, f(c)); });
        }

        /**
         * Returns true if the given rect is completely contained by the grid.
         */
        bool contains(const DiscreteRect& rect) const
        {
            return rect.x >= 0
                && rect.y >= 0
                && rect.x + rect.width <= width
                && rect.y + rect.height <= height;
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
            int x = (p.x > 0) ? p.x : 0;
            if (x >= width)
            {
                x = width - 1;
            }

            int y = (p.y > 0) ? p.y : 0;
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

            auto x1 = p.x;
            auto y1 = p.y;

            if (x1 >= width || y1 >= height)
            {
                return std::nullopt;
            }

            return get(x1, y1);
        }

        std::optional<T> tryGetValue(const Point& p) const
        {
            auto val = tryGet(p);
            if (val)
            {
                return val->get();
            }
            return std::nullopt;
        }

        template <typename U, typename Func>
        void transformAndReplace(int x, int y, const Grid<U>& replacement, Func f)
        {
            if (x + replacement.getWidth() > getWidth() || y + replacement.getHeight() > getHeight())
            {
                throw std::logic_error("replacement goes out of bounds");
            }

            replacement.forEachIndexed([&](const auto& c, const auto& v) { set(x + c.x, y + c.y, f(v)); });
        }

        template <typename U, typename Func>
        void transformAndReplace(int x, int y, int regionWidth, int regionHeight, const Grid<U>& replacement, Func f)
        {
            if (x + regionWidth > getWidth() || y + regionHeight > getHeight())
            {
                throw std::logic_error("replacement goes out of bounds");
            }
            if (regionWidth > replacement.getWidth() || regionHeight > replacement.getHeight())
            {
                throw std::logic_error("size exceeds replacement grid bounds");
            }

            GridRegion(0, 0, regionWidth, regionHeight).forEach([&](GridCoordinates c) {
                set(x + c.x, y + c.y, f(replacement.get(c.x, c.y)));
            });
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
                intersect->x, // guaranteed non-negative
                intersect->y, // guaranteed non-negative
                intersect->width,
                intersect->height);
        }
    };
}
