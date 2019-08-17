#pragma once

#include <algorithm>
#include <cassert>
#include <optional>
#include <rwe/_3do.h>

namespace rwe
{
    std::optional<_3do::Vertex> findHighestVertexOfList(const std::vector<_3do::Object>& os);

    bool compareVertexHeights(const _3do::Vertex& a, const _3do::Vertex& b);

    _3do::Vertex findHighestVertex(const _3do::Object& obj);
}
