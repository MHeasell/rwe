#pragma once

#include <optional>
#include <rwe/sim/MovementClassDefinition.h>
#include <rwe/sim/MovementClassId.h>
#include <unordered_map>

namespace rwe
{
    class MovementClassDatabase
    {
    private:
        using MovementClassMap = std::unordered_map<MovementClassId, MovementClassDefinition>;
        using ConstIterator = MovementClassMap::const_iterator;

        unsigned int nextId{0};

        std::unordered_map<std::string, MovementClassId> movementClassNameMap;
        MovementClassMap movementClassMap;

    public:
        ConstIterator begin() const;
        ConstIterator end() const;

        MovementClassId registerMovementClass(const MovementClassDefinition& definition);

        std::optional<MovementClassId> resolveMovementClassByName(const std::string& name) const;

        const MovementClassDefinition& getMovementClass(MovementClassId id) const;
    };
}
