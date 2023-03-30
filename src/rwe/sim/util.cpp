#include "util.h"
#include <rwe/util/rwe_string.h>

namespace rwe
{
    Matrix4x<SimScalar> getPieceTransform(const std::string& pieceName, const UnitModelDefinition& modelDefinition, const std::vector<UnitMesh>& pieces)
    {
        assert(modelDefinition.pieces.size() == pieces.size());

        std::optional<std::string> parentPiece = pieceName;
        auto matrix = Matrix4x<SimScalar>::identity();

        do
        {
            auto pieceIndexIt = modelDefinition.pieceIndicesByName.find(toUpper(*parentPiece));
            if (pieceIndexIt == modelDefinition.pieceIndicesByName.end())
            {
                throw std::runtime_error("missing piece definition: " + *parentPiece);
            }

            const auto& pieceDef = modelDefinition.pieces.at(pieceIndexIt->second);

            parentPiece = pieceDef.parent;

            auto pieceStateIt = pieces.begin() + pieceIndexIt->second;

            auto position = pieceDef.origin + pieceStateIt->offset;
            auto rotationX = pieceStateIt->rotationX;
            auto rotationY = pieceStateIt->rotationY;
            auto rotationZ = pieceStateIt->rotationZ;
            matrix = Matrix4x<SimScalar>::translation(position)
                * Matrix4x<SimScalar>::rotationZXY(
                    sin(rotationX),
                    cos(rotationX),
                    sin(rotationY),
                    cos(rotationY),
                    sin(rotationZ),
                    cos(rotationZ))
                * matrix;
        } while (parentPiece);

        return matrix;
    }

}
