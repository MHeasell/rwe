#ifndef RWE_WEAPONTDF_H
#define RWE_WEAPONTDF_H

#include <rwe/tdf/TdfBlock.h>
#include <string>
#include <vector>

namespace rwe
{
    class WeaponTdf
    {
    public:
        unsigned int id;
        std::string name;

        unsigned int range;

        bool ballistic;
        bool lineOfSight;
        bool dropped;
        bool vLaunch;

        bool noExplode;

        float reloadTime;
        unsigned int energyPerShot;
        unsigned int metalPerShot;
        float weaponTimer;
        bool noAutoRange;
        unsigned int weaponVelocity;
        unsigned int weaponAcceleration;
        unsigned int areaOfEffect;
        float edgeEffectiveness;

        bool turret;
        float fireStarter;
        bool unitsOnly;

        unsigned int burst;
        float burstRate;
        unsigned int sprayAngle;
        float randomDecay;

        bool groundBounce;
        float flightTime;
        bool selfProp;
        bool twoPhase;

        bool guidance;
        unsigned int turnRate;

        bool cruise;

        bool tracks;

        bool waterWeapon;

        bool burnBlow;
        unsigned int accuracy;
        unsigned int tolerance;
        unsigned int pitchTolerance;
        unsigned int aimRate;
        unsigned int holdTime;

        bool stockpile;
        bool interceptor;
        unsigned int coverage;
        bool targetable;

        bool toAirWeapon;

        unsigned int startVelocity;
        float minBarrelAngle;

        bool paralyzer;

        bool noRadar;

        std::string model;
        unsigned int color;
        unsigned int color2;
        bool smokeTrail;
        float smokeDelay;
        bool startSmoke;
        bool endSmoke;
        unsigned int renderType;
        bool beamWeapon;
        float duration;

        std::string explosionGaf;
        std::string explosionArt;

        std::string waterExplosionGaf;
        std::string waterExplosionArt;

        std::string lavaExplosionGaf;
        std::string lavaExplosionArt;

        bool propeller;

        std::string soundStart;
        std::string soundHit;
        std::string soundWater;
        bool soundTrigger;

        bool commandFire;

        unsigned int shakeMagnitude;
        float shakeDuration;

        unsigned int energy;
        unsigned int metal;

        std::vector<std::pair<std::string, unsigned int>> damage;

        std::string weaponType2;
    };

    std::vector<std::pair<std::string, unsigned int>> parseWeaponDamageBlock(const TdfBlock& block);

    WeaponTdf parseWeaponBlock(const TdfBlock& tdf);

    std::vector<std::pair<std::string, WeaponTdf>> parseWeaponTdf(const TdfBlock& tdf);
}

#endif
