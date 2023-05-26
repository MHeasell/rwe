#include "MovementClassDatabase.h"
#include <rwe/util/collection_util.h>

namespace rwe
{
    MovementClassDatabase::ConstIterator MovementClassDatabase::begin() const
    {
        return movementClassMap.begin();
    }

    MovementClassDatabase::ConstIterator MovementClassDatabase::end() const
    {
        return movementClassMap.end();
    }

    MovementClassId MovementClassDatabase::registerMovementClass(const MovementClassDefinition& definition)
    {
        MovementClassId id(nextId++);
        movementClassMap.insert({id, definition});
        movementClassNameMap.insert({definition.name, id});
        return id;
    }

    std::optional<MovementClassId> MovementClassDatabase::resolveMovementClassByName(const std::string& name) const
    {
        return tryFindValue(movementClassNameMap, name);
    }

    const MovementClassDefinition& MovementClassDatabase::getMovementClass(MovementClassId id) const
    {
        return movementClassMap.at(id);
    }
}
