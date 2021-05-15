#include "NetSchemaTdfAdapter.h"
// Special case parsing, only checks if there's a schema with type "NETWORK" (when searching all maps, a full parse and validity check is expensive)

namespace rwe
{
    void NetSchemaTdfAdapter::onStart()
    {
        hasNetSchema = false;
        schemaNestedLevel = -1; // when >= 0: we are in a schema block (or one of its children)
    }

    void NetSchemaTdfAdapter::onProperty(const std::string& name, const std::string& value)
    {
        hasNetSchema |= schemaNestedLevel >= 0 && toUpper(name) == "TYPE" && startsWith(toUpper(value), "NETWORK");
    }

    void NetSchemaTdfAdapter::onStartBlock(const std::string& name)
    {
        if (schemaNestedLevel >= 0)
        {
            ++schemaNestedLevel;
        }
        // Note: not checking for validity of the schema number (or if it even is a number). If we wanted to, we'd need to track if we're in GlobalHeader and get the SCHEMACOUNT property, and see if there's a number at the end of the schema type property...
        else if (startsWith(toUpper(name), "SCHEMA "))
        {
            schemaNestedLevel = 0;
        }
    }

    void NetSchemaTdfAdapter::onEndBlock()
    {
        if (schemaNestedLevel >= 0)
        {
            --schemaNestedLevel;
        }
    }

    NetSchemaTdfAdapter::Result NetSchemaTdfAdapter::onDone()
    {
        return hasNetSchema;
    }
}
