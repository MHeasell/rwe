#include "ListTdfAdapter.h"

namespace rwe
{
    void ListTdfAdapter::onStart()
    {
    }

    void ListTdfAdapter::onProperty(const std::string& name, const std::string& value)
    {
        blockStack.back()->insertOrAssignProperty(name, value);
    }

    void ListTdfAdapter::onStartBlock(const std::string& name)
    {
        if (blockStack.empty())
        {
            auto& block = root.emplace_back();
            blockStack.push_back(&block);
        }
        else
        {
            auto top = blockStack.back();
            auto& block = top->createBlock(name);
            blockStack.push_back(&block);
        }
    }

    void ListTdfAdapter::onEndBlock()
    {
        blockStack.pop_back();
    }

    ListTdfAdapter::Result ListTdfAdapter::onDone()
    {
        return std::move(root);
    }
}
