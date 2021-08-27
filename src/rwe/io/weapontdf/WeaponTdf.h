#pragma once

#include <rwe/io/tdf/TdfBlock.h>
#include <string>
#include <vector>

namespace rwe
{
    class WeaponTdf
    {
    public:
        int id;
        std::string name;

        int range;

        bool ballistic;
        bool lineOfSight;
        bool dropped;
        bool vLaunch;

        bool noExplode;

        float reloadTime;
        int energyPerShot;
        int metalPerShot;
        float weaponTimer;
        bool noAutoRange;
        int weaponVelocity;
        int weaponAcceleration;
        int areaOfEffect;
        float edgeEffectiveness;

        bool turret;
        float fireStarter;
        bool unitsOnly;

        int burst;
        float burstRate;
        int sprayAngle;
        float randomDecay;

        bool groundBounce;
        float flightTime;
        bool selfProp;
        bool twoPhase;

        bool guidance;
        int turnRate;

        bool cruise;

        bool tracks;

        bool waterWeapon;

        bool burnBlow;
        int accuracy;
        int tolerance;
        int pitchTolerance;
        int aimRate;
        int holdTime;

        bool stockpile;
        bool interceptor;
        int coverage;
        bool targetable;

        bool toAirWeapon;

        int startVelocity;
        float minBarrelAngle;

        bool paralyzer;

        bool noRadar;

        std::string model;
        int color;
        int color2;
        bool smokeTrail;
        float smokeDelay;
        bool startSmoke;
        bool endSmoke;
        int renderType;
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

        int shakeMagnitude;
        float shakeDuration;

        int energy;
        int metal;

        std::vector<std::pair<std::string, int>> damage;

        std::string weaponType2;
    };

    std::vector<std::pair<std::string, int>> parseWeaponDamageBlock(const TdfBlock& block);

    WeaponTdf parseWeaponBlock(const TdfBlock& tdf);

    std::vector<std::pair<std::string, WeaponTdf>> parseWeaponTdf(const TdfBlock& tdf);
}
