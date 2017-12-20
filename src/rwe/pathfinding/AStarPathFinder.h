#ifndef RWE_ASTARPATHFINDER_H
#define RWE_ASTARPATHFINDER_H

#include <boost/optional.hpp>
#include <deque>
#include <rwe/MinHeap.h>
#include <unordered_map>
#include <vector>

namespace rwe
{
    template <typename T>
    struct AStarVertexInfo
    {
        float costToReach;
        T vertex;
        boost::optional<const AStarVertexInfo<T>*> predecessor;
    };

    template <typename T>
    class AStarPathFinder
    {
    public:
        using VertexInfo = AStarVertexInfo<T>;

    public:
        std::vector<T> findPath(const T& start, const T& goal)
        {
            auto openVertices = createMinHeap<T, std::pair<float, VertexInfo>>(
                [](const auto& p) { return p.second.vertex; },
                [](const auto& a, const auto& b) { return a.first < b.first; });
            openVertices.pushOrDecrease({estimateCost(start, goal), VertexInfo{0.0f, start, boost::none}});

            std::unordered_map<T, VertexInfo> closedVertices;

            while (!openVertices.empty())
            {
                const std::pair<float, VertexInfo>& openFront = openVertices.top();
                const VertexInfo& current = closedVertices[openFront.second.vertex] = openFront.second;
                openVertices.pop();

                if (current.vertex == goal)
                {
                    return walkPath(current);
                }

                for (const VertexInfo& s : getSuccessors(current))
                {
                    if (closedVertices.find(s.vertex) != closedVertices.end())
                    {
                        continue;
                    }

                    auto estimatedTotalCost = s.costToReach + estimateCost(s.vertex, goal);
                    openVertices.pushOrDecrease({estimatedTotalCost, s});
                }
            }

            throw std::logic_error("Path not found, and I haven't figured out what to do about this case yet");
        }

    protected:
        virtual float estimateCost(const T& start, const T& goal) = 0;

        virtual std::vector<VertexInfo> getSuccessors(const VertexInfo& vertex) = 0;

    private:
        std::vector<T> walkPath(const VertexInfo& info)
        {
            std::vector<T> items;
            boost::optional<const VertexInfo*> v = &info;
            while (v)
            {
                items.push_back((*v)->vertex);
                v = (*v)->predecessor;
            }

            std::reverse(items.begin(), items.end());
            return items;
        }
    };
}

#endif
