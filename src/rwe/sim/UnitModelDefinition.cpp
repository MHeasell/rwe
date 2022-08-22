#include "UnitModelDefinition.h"
#include <rwe/Index.h>
#include <rwe/rwe_string.h>

namespace rwe
{
    std::unordered_map<std::string, int> createPieceNameIndex(const std::vector<UnitPieceDefinition>& pieces)
    {
        std::unordered_map<std::string, int> m;
        for (Index i = 0; i < getSize(pieces); ++i)
        {
            m.insert_or_assign(toUpper(pieces[i].name), i);
        }
        return m;
    }

    UnitModelDefinition createUnitModelDefinition(SimScalar height, std::vector<UnitPieceDefinition>&& pieces)
    {
        UnitModelDefinition d;
        d.height = height;
        d.pieces = std::move(pieces);
        d.pieceIndicesByName = createPieceNameIndex(d.pieces);
        return d;
    }
}
