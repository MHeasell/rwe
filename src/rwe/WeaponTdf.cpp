#include "WeaponTdf.h"

namespace rwe
{
    std::vector<std::pair<std::string, unsigned int>> parseWeaponDamageBlock(const TdfBlock& block)
    {
        std::vector<std::pair<std::string, unsigned int>> items;

        for (const auto& entry : block.entries)
        {
            auto value = boost::get<std::string>(entry.value.get());
            if (value)
            {
                items.emplace_back(entry.name, tdfTryParse<unsigned int>(*value));
            }
        }

        return items;
    }

    WeaponTdf parseWeaponBlock(const TdfBlock& tdf)
    {
        WeaponTdf w;

        tdf.readOrDefault("id", w.id);
        tdf.readOrDefault("name", w.name);

        tdf.readOrDefault("range", w.range);

        tdf.readOrDefault("ballistic", w.ballistic);
        tdf.readOrDefault("lineOfSight", w.lineOfSight);
        tdf.readOrDefault("dropped", w.dropped);
        tdf.readOrDefault("vLaunch", w.vLaunch);

        tdf.readOrDefault("noExplode", w.noExplode);

        tdf.readOrDefault("reloadTime", w.reloadTime);
        tdf.readOrDefault("energyPerShot", w.energyPerShot);
        tdf.readOrDefault("metalPerShot", w.metalPerShot);
        tdf.readOrDefault("weaponTimer", w.weaponTimer);
        tdf.readOrDefault("noAutoRange", w.noAutoRange);
        tdf.readOrDefault("weaponVelocity", w.weaponVelocity);
        tdf.readOrDefault("weaponAcceleration", w.weaponAcceleration);
        tdf.readOrDefault("areaOfEffect", w.areaOfEffect);
        tdf.readOrDefault("edgeEffectiveness", w.edgeEffectiveness);

        tdf.readOrDefault("turret", w.turret);
        tdf.readOrDefault("fireStarter", w.fireStarter);
        tdf.readOrDefault("unitsOnly", w.unitsOnly);

        tdf.readOrDefault("burst", w.burst);
        tdf.readOrDefault("burstRate", w.burstRate);
        tdf.readOrDefault("sprayAngle", w.sprayAngle);
        tdf.readOrDefault("randomDecay", w.randomDecay);

        tdf.readOrDefault("groundBounce", w.groundBounce);
        tdf.readOrDefault("flightTime", w.flightTime);
        tdf.readOrDefault("selfProp", w.selfProp);
        tdf.readOrDefault("twoPhase", w.twoPhase);

        tdf.readOrDefault("guidance", w.guidance);
        tdf.readOrDefault("turnRate", w.turnRate);

        tdf.readOrDefault("cruise", w.cruise);

        tdf.readOrDefault("tracks", w.tracks);

        tdf.readOrDefault("waterWeapon", w.waterWeapon);

        tdf.readOrDefault("burnBlow", w.burnBlow);
        tdf.readOrDefault("accuracy", w.accuracy);
        tdf.readOrDefault("tolerance", w.tolerance);
        tdf.readOrDefault("pitchTolerance", w.pitchTolerance);
        tdf.readOrDefault("aimRate", w.aimRate);
        tdf.readOrDefault("holdTime", w.holdTime);

        tdf.readOrDefault("stockpile", w.stockpile);
        tdf.readOrDefault("interceptor", w.interceptor);
        tdf.readOrDefault("coverage", w.coverage);
        tdf.readOrDefault("targetable", w.targetable);

        tdf.readOrDefault("toAirWeapon", w.toAirWeapon);

        tdf.readOrDefault("startVelocity", w.startVelocity);
        tdf.readOrDefault("minBarrelAngle", w.minBarrelAngle);

        tdf.readOrDefault("paralyzer", w.paralyzer);

        tdf.readOrDefault("noRadar", w.noRadar);

        tdf.readOrDefault("model", w.model);
        tdf.readOrDefault("color", w.color);
        tdf.readOrDefault("color2", w.color2);
        tdf.readOrDefault("smokeTrail", w.smokeTrail);
        tdf.readOrDefault("smokeDelay", w.smokeDelay);
        tdf.readOrDefault("startSmoke", w.startSmoke);
        tdf.readOrDefault("endSmoke", w.endSmoke);
        tdf.readOrDefault("renderType", w.renderType);
        tdf.readOrDefault("beamWeapon", w.beamWeapon);


        tdf.readOrDefault("explosionGaf", w.explosionGaf);
        tdf.readOrDefault("explosionArt", w.explosionArt);

        tdf.readOrDefault("waterExplosionGaf", w.waterExplosionGaf);
        tdf.readOrDefault("waterExplosionArt", w.waterExplosionArt);

        tdf.readOrDefault("lavaExplosionGaf", w.lavaExplosionGaf);
        tdf.readOrDefault("lavaExplosionArt", w.lavaExplosionArt);

        tdf.readOrDefault("propeller", w.propeller);

        tdf.readOrDefault("soundStart", w.soundStart);
        tdf.readOrDefault("soundHit", w.soundHit);
        tdf.readOrDefault("soundWater", w.soundWater);
        tdf.readOrDefault("soundTrigger", w.soundTrigger);

        tdf.readOrDefault("commandFire", w.commandFire);

        tdf.readOrDefault("shakeMagnitude", w.shakeMagnitude);
        tdf.readOrDefault("shakeDuration", w.shakeDuration);

        tdf.readOrDefault("energy", w.energy);
        tdf.readOrDefault("metal", w.metal);

        auto damageBlock = tdf.findBlock("DAMAGE");
        if (damageBlock)
        {
            w.damage = parseWeaponDamageBlock(*damageBlock);
        }

        tdf.readOrDefault("weaponType2", w.weaponType2);

        return w;
    }

    std::vector<std::pair<std::string, WeaponTdf>> parseWeaponTdf(const TdfBlock& tdf)
    {
        std::vector<std::pair<std::string, WeaponTdf>> items;
        for (const auto& entry : tdf.entries)
        {
            const auto& weaponName = entry.name;
            const auto block = boost::get<TdfBlock>(entry.value.get());
            if (block != nullptr)
            {
                items.emplace_back(weaponName, parseWeaponBlock(*block));
            }
        }
        return std::vector<std::pair<std::string, WeaponTdf>>();
    }
}
