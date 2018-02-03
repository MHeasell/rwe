#include "SimpleTdfAdapter.h"

namespace rwe
{
    void SimpleTdfAdapter::onStart()
    {
        root.properties.clear();
        root.blocks.clear();
        blockStack.clear();
        blockStack.push_back(&root);
    }

    void SimpleTdfAdapter::onProperty(const std::string& name, const std::string& value)
    {
        blockStack.back()->insertOrAssignProperty(name, value);
    }

    void SimpleTdfAdapter::onStartBlock(const std::string& name)
    {
        auto top = blockStack.back();
        auto& block = top->createBlock(name);
        blockStack.push_back(&block);
    }

    void SimpleTdfAdapter::onEndBlock()
    {
        blockStack.pop_back();
    }

    SimpleTdfAdapter::Result SimpleTdfAdapter::onDone()
    {
        return std::move(root);
    }
}
