#pragma once

#include <deque>
#include <rwe/ColorPalette.h>
#include <rwe/game/WeaponMediaInfo.h>
#include <rwe/io/cob/Cob.h>
#include <rwe/io/fbi/UnitFbi.h>
#include <rwe/io/moveinfotdf/MovementClassTdf.h>
#include <rwe/io/tnt/TntArchive.h>
#include <rwe/io/weapontdf/WeaponTdf.h>
#include <rwe/sim/FeatureDefinitionId.h>
#include <rwe/sim/MapTerrain.h>
#include <rwe/sim/MovementClassCollisionService.h>
#include <rwe/sim/MovementClassDatabase.h>
#include <rwe/sim/UnitDefinition.h>
#include <rwe/sim/WeaponDefinition.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <unordered_map>

namespace rwe
{
    MovementClassCollisionService createMovementClassCollisionService(const MapTerrain& terrain, const MovementClassDatabase& movementClassDatabase);

    std::unordered_map<std::string, CobScript> loadCobScripts(AbstractVirtualFileSystem& vfs);

    std::vector<std::string> getFeatureNames(TntArchive& tnt);

    Grid<std::size_t> getMapData(TntArchive& tnt);

    Grid<unsigned char> getHeightGrid(const Grid<TntTileAttributes>& attrs);

    Vector3f colorToVector(const Color& color);

    unsigned int colorDistance(const Color& a, const Color& b);

    Vector3f getLaserColorUtil(const std::vector<Color>& palette, const std::vector<Color>& guiPalette, unsigned int colorIndex);

    std::optional<std::string> getFxName(unsigned int code);

    MovementClassDefinition parseMovementClassDefinition(const MovementClassTdf& tdf);

    WeaponDefinition parseWeaponDefinition(const WeaponTdf& tdf);

    std::optional<YardMapCell> parseYardMapCell(char c);

    std::vector<YardMapCell> parseYardMapCells(const std::string& yardMap);

    Grid<YardMapCell> parseYardMap(unsigned int width, unsigned int height, const std::string& yardMap);

    UnitDefinition parseUnitDefinition(const UnitFbi& fbi, MovementClassDatabase& movementClassDatabase);

    WeaponMediaInfo parseWeaponMediaInfo(const std::vector<Color>& palette, const std::vector<Color>& guiPalette, const WeaponTdf& tdf);

    FeatureDefinitionId getFeatureId(FeatureDefinitionId& nextId, const std::unordered_map<std::string, FeatureDefinitionId>& featureNameIndex, std::deque<std::string>& openQueue, std::unordered_map<std::string, FeatureDefinitionId>& openSet, const std::string& featureName);
}
