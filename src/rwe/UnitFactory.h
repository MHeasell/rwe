#pragma once

#include <rwe/MeshService.h>
#include <rwe/MovementClassCollisionService.h>
#include <rwe/UnitDatabase.h>
#include <rwe/sim/MovementClass.h>
#include <rwe/sim/SimVector.h>
#include <rwe/sim/Unit.h>
#include <string>

namespace rwe
{
    class UnitFactory
    {
    private:
        TextureService* const textureService;
        UnitDatabase* const unitDatabase;
        MeshService meshService;
        MovementClassCollisionService* const collisionService;
        const ColorPalette* palette;
        const ColorPalette* guiPalette;

    public:
        UnitFactory(
            TextureService* textureService,
            UnitDatabase* unitDatabase,
            MeshService&& meshService,
            MovementClassCollisionService* collisionService,
            const ColorPalette* palette,
            const ColorPalette* guiPalette);

    public:
        Unit createUnit(const std::string& unitType, PlayerId owner, const SimVector& position, std::optional<const std::reference_wrapper<SimAngle>> rotation);

        std::optional<std::reference_wrapper<const std::vector<GuiEntry>>> getBuilderGui(const std::string& unitType, int page) const;

        /** If the unit has no build gui, this will be zero. */
        int getBuildPageCount(const std::string& unitType) const;

        Point getUnitFootprint(const std::string& unitType) const;

        MovementClass getAdHocMovementClass(const std::string& unitType) const;

        bool isValidUnitType(const std::string& unitType) const;

    private:
        std::optional<UnitWeapon> tryCreateWeapon(const std::string& weaponType);
        UnitWeapon createWeapon(const std::string& weaponType);
        UnitWeapon createWeapon(const std::string& weaponType, const WeaponTdf& tdf);

        Vector3f getLaserColor(unsigned int colorIndex);
    };
}
