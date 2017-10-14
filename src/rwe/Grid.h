#ifndef RWE_GRID_H
#define RWE_GRID_H

#include <vector>
#include <cassert>
#include <stdexcept>
#include <functional>

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

        template <typename U>
        void transformAndReplaceArea(std::size_t x, std::size_t y, const Grid<U>& replacement, const std::function<T(const U&)>& transformation);
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
        if (replacement.getWidth() > getWidth() - x || replacement.getHeight() > getHeight() - x)
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
        if (replacement.getWidth() > getWidth() - x || replacement.getHeight() > getHeight() - x)
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
}

#endif
