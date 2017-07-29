#include "SimpleTdfAdapter.h"

namespace rwe
{

    void SimpleTdfAdapter::onStart()
    {
        blockStack.push(&root);
    }

    void SimpleTdfAdapter::onProperty(const std::string& name, const std::string& value)
    {
        blockStack.top()->emplace_back(name, value);
    }

    void SimpleTdfAdapter::onStartBlock(const std::string& name)
    {
        auto top = blockStack.top();
        top->emplace_back(name);
        blockStack.push(&boost::get<std::vector<BlockEntry>>(*top->back().value));
    }

    void SimpleTdfAdapter::onEndBlock()
    {
        blockStack.pop();
    }

    const std::vector<SimpleTdfAdapter::BlockEntry>& SimpleTdfAdapter::getRoot() const
    {
        return root;
    }

    void SimpleTdfAdapter::onDone()
    {
    }

    std::ostream& operator<<(std::ostream& os, const SimpleTdfAdapter::BlockEntry& entry)
    {
        auto leaf = boost::get<std::string>(&*entry.value);
        if (leaf != nullptr)
        {
            os << entry.name << " = " << *leaf << ";";
        }
        else
        {
            auto block = boost::get<std::vector<SimpleTdfAdapter::BlockEntry>>(&*entry.value);
            if (block != nullptr)
            {
                os << "[" << entry.name << "]{ ";
                for (auto elem : *block)
                {
                    os << elem << " ";
                }
                os << "}";
            }
            else
            {
                throw std::logic_error("Value was neither primitive nor block");
            }
        }

        return os;
    }
}
