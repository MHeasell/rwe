#ifndef RWE_ASTARPATHFINDER_H
#define RWE_ASTARPATHFINDER_H

#include <boost/optional.hpp>
#include <deque>
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
            std::deque<std::pair<float, VertexInfo>> openVertices{
                {estimateCost(start, goal), VertexInfo{0.0f, start, boost::none}}};
            std::unordered_map<T, VertexInfo> closedVertices;

            while (!openVertices.empty())
            {
                const std::pair<float, VertexInfo>& openFront = openVertices.front();
                const VertexInfo& current = closedVertices[openFront.second.vertex] = openFront.second;
                openVertices.pop_front();

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

                    const auto& existingIt = std::find_if(openVertices.begin(),
                        openVertices.end(),
                        [&s](const auto& pair) { return pair.second.vertex == s.vertex; });

                    if (existingIt == openVertices.end())
                    {
                        addEntry(openVertices, {estimatedTotalCost, s});
                        continue;
                    }

                    if (estimatedTotalCost >= existingIt->first)
                    {
                        continue;
                    }

                    openVertices.erase(existingIt);
                    addEntry(openVertices, {estimatedTotalCost, s});
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

        void addEntry(std::deque<std::pair<float, VertexInfo>>& openList, const std::pair<float, VertexInfo>& pair)
        {
            auto it = std::find_if(openList.begin(), openList.end(), [&pair](const auto& p){ return p.first >= pair.first; });
            openList.insert(it, pair);
        }
    };
}

#endif
