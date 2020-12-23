#include "GameScene_util.h"

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>

namespace rwe
{
    Matrix4x<SimScalar> getPieceTransform(const std::string& pieceName, const std::vector<UnitPieceDefinition>& pieceDefinitions, const std::vector<UnitMesh>& pieces)
    {
        assert(pieceDefinitions.size() == pieces.size());

        std::optional<std::string> parentPiece = pieceName;
        auto matrix = Matrix4x<SimScalar>::identity();

        do
        {
            auto pieceDefIt = std::find_if(pieceDefinitions.begin(), pieceDefinitions.end(), [&](const auto& p) { return boost::iequals(p.name, *parentPiece); });
            if (pieceDefIt == pieceDefinitions.end())
            {
                throw std::runtime_error("missing piece definition: " + *parentPiece);
            }

            parentPiece = pieceDefIt->parent;

            auto pieceStateIt = pieces.begin() + (pieceDefIt - pieceDefinitions.begin());

            auto position = pieceDefIt->origin + pieceStateIt->offset;
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
