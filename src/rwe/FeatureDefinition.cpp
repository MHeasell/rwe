#include "FeatureDefinition.h"

#include <rwe/tdf.h>

namespace rwe
{
    FeatureDefinition FeatureDefinition::fromTdf(const TdfBlock& tdf)
    {
        FeatureDefinition f;

        std::string emptyString;

        f.world = tdf.findValue("world").get_value_or(emptyString);
        f.description = tdf.findValue("description").get_value_or(emptyString);
        f.category = tdf.findValue("category").get_value_or(emptyString);

        f.footprintX = extractUint(tdf, "footprintx").get_value_or(1);
        f.footprintZ = extractUint(tdf, "footprintz").get_value_or(1);
        f.height = extractUint(tdf, "height").get_value_or(0);

        f.animating = extractBool(tdf, "animating").get_value_or(false);
        f.fileName = tdf.findValue("filename").get_value_or(emptyString);
        f.seqName = tdf.findValue("seqname").get_value_or(emptyString);
        f.animTrans = extractBool(tdf, "animtrans").get_value_or(false);
        f.seqNameShad = tdf.findValue("seqnameshad").get_value_or(emptyString);
        f.shadTrans = extractBool(tdf, "shadtrans").get_value_or(false);

        f.object = tdf.findValue("object").get_value_or(emptyString);

        f.reclaimable = extractBool(tdf, "reclaimable").get_value_or(false);
        f.autoreclaimable = extractBool(tdf, "autoreclaimable").get_value_or(true);
        f.seqNameReclamate = tdf.findValue("seqnamereclamate").get_value_or(emptyString);
        f.featureReclamate = tdf.findValue("featurereclamate").get_value_or(emptyString);
        f.metal = extractUint(tdf, "metal").get_value_or(0);
        f.energy = extractUint(tdf, "energy").get_value_or(0);

        f.flamable = extractBool(tdf, "flamable").get_value_or(false);
        f.seqNameBurn = tdf.findValue("seqnameburn").get_value_or(emptyString);
        f.seqNameBurnShad = tdf.findValue("seqnameburnshad").get_value_or(emptyString);
        f.featureBurnt = tdf.findValue("featureburnt").get_value_or(emptyString);
        f.burnMin = extractUint(tdf, "burnmin").get_value_or(0);
        f.burnMax = extractUint(tdf, "burnmax").get_value_or(0);
        f.sparkTime = extractUint(tdf, "sparktime").get_value_or(0);
        f.spreadChance = extractUint(tdf, "spreadchance").get_value_or(0);
        f.burnWeapon = tdf.findValue("burnweapon").get_value_or(emptyString);

        f.geothermal = extractBool(tdf, "geothermal").get_value_or(false);

        f.hitDensity = extractUint(tdf, "hitdensity").get_value_or(0);

        f.reproduce = extractBool(tdf, "reproduce").get_value_or(false);
        f.reproduceArea = extractUint(tdf, "reproducearea").get_value_or(0);

        f.noDisplayInfo = extractBool(tdf, "nodisplayinfo").get_value_or(false);

        f.permanent = extractBool(tdf, "permanent").get_value_or(false);

        f.blocking = extractBool(tdf, "blocking").get_value_or(true);

        f.indestructible = extractBool(tdf, "indestructible").get_value_or(false);
        f.damage = extractUint(tdf, "damage").get_value_or(1);
        f.seqNameDie = tdf.findValue("seqnamedie").get_value_or(emptyString);
        f.featureDead = tdf.findValue("featuredead").get_value_or(emptyString);

        return f;
    }
}
