#ifndef RWE_GRID_H
#define RWE_GRID_H

#include <cassert>
#include <functional>
#include <optional>
#include <rwe/DiscreteRect.h>
#include <rwe/GridRegion.h>
#include <stdexcept>
#include <vector>

namespace rwe
{
    template <typename T>
    class Grid
    {
    private:
        std::size_t width;
        std::size_t height;
        std::vector<T> data;

    public:
        Grid();
        Grid(std::size_t width, std::size_t height);
        Grid(std::size_t width, std::size_t height, const T& initialValue);
        Grid(std::size_t width, std::size_t height, std::vector<T>&& data);

        T& get(std::size_t x, std::size_t y);
        const T& get(std::size_t x, std::size_t y) const;

        void set(std::size_t x, std::size_t y, const T& value);
        void set(std::size_t x, std::size_t y, T&& value);

        bool operator==(const Grid& rhs) const;

        bool operator!=(const Grid& rhs) const;

        std::size_t toIndex(std::size_t x, std::size_t y) const;

        std::size_t getWidth() const;
        std::size_t getHeight() const;

        const T* getData() const;
        T* getData();

        const std::vector<T> getVector() const;
        std::vector<T> getVector();

        void replaceArea(std::size_t x, std::size_t y, const Grid<T>& replacement);

        void setArea(std::size_t x, std::size_t y, std::size_t width, std::size_t height, const T& value);

        void setArea(const GridRegion& region, const T& value);

        /**
         * Returns true if the given rect is completely contained by the grid.
         */
        bool contains(const DiscreteRect& rect) const;

        /**
         * Converts the given discrete rect into a grid region
         * as long as it is contained entirely within the grid.
         */
        std::optional<GridRegion> tryToRegion(const DiscreteRect& rect) const;

        /**
         * If the given point is within the bounds of the grid
         * returns the value at the corresponding grid cell.
         */
        std::optional<std::reference_wrapper<const T>> tryGet(const Point& p) const;

        template <typename U>
        void transformAndReplaceArea(std::size_t x, std::size_t y, const Grid<U>& replacement, const std::function<T(const U&)>& transformation);

        /**
         * Returns a clipped version of region
         * representing the intersection of region
         * with the grid bounds.
         * That is, the returned region is will be the portion of the input
         * that lies inside the grid.
         */
        GridRegion clipRegion(const DiscreteRect& region) const;
    };

    template <typename T>
    Grid<T>::Grid() : width(0), height(0)
    {
    }

    template <typename T>
    Grid<T>::Grid(std::size_t width, std::size_t height)
        : width(width), height(height), data(width * height)
    {
    }

    template <typename T>
    Grid<T>::Grid(std::size_t width, std::size_t height, const T& initialValue)
        : width(width), height(height), data(width * height, initialValue)
    {
    }

    template <typename T>
    Grid<T>::Grid(std::size_t width, std::size_t height, std::vector<T>&& data)
        : width(width), height(height), data(std::move(data))
    {
        assert(this->data.size() == width * height);
    }

    template <typename T>
    bool Grid<T>::operator==(const Grid& rhs) const
    {
        return width == rhs.width && height == rhs.height && data == rhs.data;
    }

    template <typename T>
    bool Grid<T>::operator!=(const Grid& rhs) const
    {
        return !(rhs == *this);
    }

    template <typename T>
    T& Grid<T>::get(std::size_t x, std::size_t y)
    {
        return data[toIndex(x, y)];
    }

    template <typename T>
    const T& Grid<T>::get(std::size_t x, std::size_t y) const
    {
        return data[toIndex(x, y)];
    }

    template <typename T>
    void Grid<T>::set(std::size_t x, std::size_t y, const T& value)
    {
        data[toIndex(x, y)] = value;
    }

    template <typename T>
    void Grid<T>::set(std::size_t x, std::size_t y, T&& value)
    {
        data[toIndex(x, y)] = std::move(value);
    }

    template <typename T>
    std::size_t Grid<T>::toIndex(std::size_t x, std::size_t y) const
    {
        assert(x < width && y < height);
        return (y * width) + x;
    }

    template <typename T>
    std::size_t Grid<T>::getWidth() const
    {
        return width;
    }

    template <typename T>
    std::size_t Grid<T>::getHeight() const
    {
        return height;
    }

    template <typename T>
    const T* Grid<T>::getData() const
    {
        return data.data();
    }

    template <typename T>
    T* Grid<T>::getData()
    {
        return data.data();
    }

    template <typename T>
    const std::vector<T> Grid<T>::getVector() const
    {
        return data;
    }

    template <typename T>
    std::vector<T> Grid<T>::getVector()
    {
        return data;
    }

    template <typename T>
    void Grid<T>::replaceArea(std::size_t x, std::size_t y, const Grid<T>& replacement)
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

    template <typename T>
    template <typename U>
    void Grid<T>::transformAndReplaceArea(std::size_t x, std::size_t y, const Grid<U>& replacement, const std::function<T(const U&)>& transformation)
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

    template <typename T>
    void Grid<T>::setArea(std::size_t x, std::size_t y, std::size_t width, std::size_t height, const T& value)
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

    template <typename T>
    void Grid<T>::setArea(const GridRegion& region, const T& value)
    {
        setArea(region.x, region.y, region.width, region.height, value);
    }

    template <typename T>
    GridRegion Grid<T>::clipRegion(const DiscreteRect& rect) const
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

    template <typename T>
    bool Grid<T>::contains(const DiscreteRect& rect) const
    {
        return rect.x >= 0
            && rect.y >= 0
            && rect.x + static_cast<int>(rect.width) <= width
            && rect.y + static_cast<int>(rect.height) <= height;
    }

    template <typename T>
    std::optional<GridRegion> Grid<T>::tryToRegion(const DiscreteRect& rect) const
    {
        if (!contains(rect))
        {
            return std::nullopt;
        }

        return GridRegion(rect.x, rect.y, rect.width, rect.height);
    }

    template <typename T>
    std::optional<std::reference_wrapper<const T>> Grid<T>::tryGet(const Point& p) const
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
}

#endif
