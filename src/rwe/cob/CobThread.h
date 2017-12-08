#ifndef RWE_COBTHREAD_H
#define RWE_COBTHREAD_H

#include <boost/variant.hpp>
#include <rwe/util.h>
#include <stack>
#include <vector>

namespace rwe
{
    class CobEnvironment;

    class CobThread
    {
    public:
        std::string name;

        std::stack<int> stack;
        std::stack<std::vector<int>> locals;
        unsigned int instructionIndex{0};

        CobThread(
            const std::string& name,
            unsigned int instructionIndex,
            unsigned int endIndex,
            const std::vector<int>& params);
    };
}

#endif
