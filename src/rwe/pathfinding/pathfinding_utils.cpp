#include "pathfinding_utils.h"

namespace rwe
{
    std::vector<PathVertex> runSimplifyPath(const std::vector <PathVertex>& input)
    {
        std::vector<PathVertex> output(input.size());
        auto count = simplifyPath(input.begin(), input.end(), output.begin());
        output.resize(count);
        return output;
    }
}
