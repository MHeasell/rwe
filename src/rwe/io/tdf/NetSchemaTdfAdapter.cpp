#include "NetSchemaTdfAdapter.h"
// Special case parsing, only checks if there's a schema with type "NETWORK" (when searching all maps, a full parse and validity check is expensive)

namespace rwe
{
    void NetSchemaTdfAdapter::onStart()
    {
        bHasNetSchema = false;
        bInSchema = false;
    }

    void NetSchemaTdfAdapter::onProperty(const std::string& name, const std::string& value)
    {
        // TODO (kwh) - ok ok this is ridiculous overkill. gives about 2.5% boost to loading the map list
        bHasNetSchema |= bInSchema && name.size() == 4 && (name[0] == 'T' || name[0] == 't') && (name[1] == 'y' || name[1] == 'Y') && (name[2] == 'p' || name[2] == 'P') && (name[3] == 'e' || name[3] == 'E') && value.size() >= 7 && (value[0] == 'N' || value[0] == 'n') && (value[1] == 'e' || value[1] == 'E') && (value[2] == 't' || value[2] == 'T') && (value[3] == 'w' || value[3] == 'W') && (value[4] == 'o' || value[4] == 'O') && (value[5] == 'r' || value[5] == 'R') && (value[6] == 'k' || value[6] == 'K');
        //bHasNetSchema |= bInSchema && toUpper(name) == "TYPE" && startsWith(toUpper(value), "NETWORK");
    }

    void NetSchemaTdfAdapter::onStartBlock(const std::string& name)
    {
        // Note: not checking for validity of the schema number (or if it even is a number). If we wanted to, we'd need to track if we're in GlobalHeader and get the SCHEMACOUNT property, and see if there's a number at the end of the schema type property...
        bInSchema |= name.size() > 7 && (name[0] == 'S' || name[0] == 's') && (name[1] == 'c' || name[1] == 'C') && (name[2] == 'h' || name[2] == 'H') && (name[3] == 'e' || name[3] == 'E') && (name[4] == 'm' || name[4] == 'M') && (name[5] == 'a' || name[5] == 'A') && name[6] == ' ';
        //bInSchema |= startsWith(toUpper(name), "SCHEMA "); 
    }

    void NetSchemaTdfAdapter::onEndBlock()
    {
        bInSchema = false;
    }

    NetSchemaTdfAdapter::Result NetSchemaTdfAdapter::onDone()
    {
        return bHasNetSchema;
    }
}
