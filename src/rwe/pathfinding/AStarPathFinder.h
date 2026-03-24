#pragma once

#include <deque>
#include <algorithm>
#include <optional>
#include <rwe/collections/MinHeap.h>
#include <rwe/util/SimpleLogger.h>
#include <unordered_map>
#include <vector>

namespace rwe
{
    /**
     * The maximum number of elements in the open list to expand
     * before giving up on a path search.
     */
    const unsigned int MaxOpenListQueries = 1000;

    template <typename T, typename Cost = float>
    struct AStarVertexInfo
    {
        Cost costToReach;
        T vertex;
        std::optional<const AStarVertexInfo<T, Cost>*> predecessor;
    };

    enum class AStarPathType
    {
        Complete,
        Partial
    };

    template <typename T, typename Cost>
    struct AStarPathInfo
    {
        AStarPathType type;
        std::vector<T> path;
        std::unordered_map<T, AStarVertexInfo<T, Cost>> closedVertices;
    };

    template <typename T, typename Cost = float>
    class AStarPathFinder
    {
    public:
        using VertexInfo = AStarVertexInfo<T, Cost>;

    public:
        AStarPathInfo<T, Cost> findPath(const T& start)
        {
            auto openVertices = createMinHeap<T, std::pair<Cost, VertexInfo>>(
                [](const auto& p) { return p.second.vertex; },
                [](const auto& a, const auto& b) { return a.first < b.first; });
            openVertices.pushOrDecrease({estimateCostToGoal(start), VertexInfo{Cost(), start, std::nullopt}});

            std::unordered_map<T, VertexInfo> closedVertices;

            std::optional<std::pair<Cost, const VertexInfo*>> closestVertex;

            unsigned int openListPopsPerformed = 0;

            while (!openVertices.empty() && openListPopsPerformed < MaxOpenListQueries)
            {
                const std::pair<Cost, VertexInfo>& openFront = openVertices.top();
                const VertexInfo& current = closedVertices[openFront.second.vertex] = openFront.second;
                openVertices.pop();
                openListPopsPerformed += 1;

                if (isGoal(current.vertex))
                {
                    LOG_DEBUG << "Found goal after visiting " << openListPopsPerformed << " vertices";
                    return AStarPathInfo<T, Cost>{AStarPathType::Complete, walkPath(current), std::move(closedVertices)};
                }

                auto estimatedCostToGoal = estimateCostToGoal(current.vertex);
                if (!closestVertex || estimatedCostToGoal < closestVertex->first)
                {
                    closestVertex = std::pair<Cost, const VertexInfo*>(estimatedCostToGoal, &current);
                }

                for (const VertexInfo& s : getSuccessors(current))
                {
                    if (closedVertices.find(s.vertex) != closedVertices.end())
                    {
                        continue;
                    }

                    auto estimatedTotalCost = s.costToReach + estimateCostToGoal(s.vertex);
                    openVertices.pushOrDecrease({estimatedTotalCost, s});
                }
            }

            LOG_DEBUG << "Failed to find goal, visited " << openListPopsPerformed << " vertices";
            return AStarPathInfo<T, Cost>{AStarPathType::Partial, walkPath(*(closestVertex->second)), std::move(closedVertices)};
        }

    protected:
        virtual bool isGoal(const T& vertex) = 0;

        virtual Cost estimateCostToGoal(const T& vertex) = 0;

        virtual std::vector<VertexInfo> getSuccessors(const VertexInfo& vertex) = 0;

    private:
        std::vector<T> walkPath(const VertexInfo& info)
        {
            std::vector<T> items;
            std::optional<const VertexInfo*> v = &info;
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
