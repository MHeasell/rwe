#include "Projectile.h"

namespace rwe
{
    SimVector Projectile::getBackPosition(const ProjectileRenderTypeLaser& laserRenderType) const
    {
        auto durationVector = velocity * laserRenderType.duration;
        if (durationVector.lengthSquared() < (position - origin).lengthSquared())
        {
            return position - durationVector;
        }
        else
        {
            return origin;
        }
    }

    unsigned int Projectile::getDamage(const std::string& unitType) const
    {
        auto it = damage.find(unitType);
        if (it != damage.end())
        {
            return it->second;
        }

        it = damage.find("DEFAULT");
        if (it != damage.end())
        {
            return it->second;
        }

        throw std::runtime_error("Failed to find damage entry for projectile");
    }
}
