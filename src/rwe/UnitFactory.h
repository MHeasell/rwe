#ifndef RWE_UNITFACTORY_H
#define RWE_UNITFACTORY_H

#include <rwe/MeshService.h>
#include <rwe/MovementClass.h>
#include <rwe/MovementClassCollisionService.h>
#include <rwe/Unit.h>
#include <rwe/UnitDatabase.h>
#include <string>

namespace rwe
{
    class UnitFactory
    {
    private:
        TextureService* const textureService;
        UnitDatabase unitDatabase;
        MeshService meshService;
        MovementClassCollisionService* const collisionService;
        const ColorPalette* palette;
        const ColorPalette* guiPalette;

    public:
        UnitFactory(
            TextureService* textureService,
            UnitDatabase&& unitDatabase,
            MeshService&& meshService,
            MovementClassCollisionService* collisionService,
            const ColorPalette* palette,
            const ColorPalette* guiPalette);

    public:
        Unit createUnit(const std::string& unitType, PlayerId owner, unsigned int colorIndex, const Vector3f& position);

        std::optional<std::reference_wrapper<const std::vector<GuiEntry>>> getBuilderGui(const std::string& unitType, unsigned int page) const;

    private:
        UnitWeapon createWeapon(const std::string& weaponType);

        Vector3f getLaserColor(unsigned int colorIndex);
    };
}

#endif
