#include "FeatureDefinition.h"

namespace rwe
{
    FeatureDefinition FeatureDefinition::fromTdf(const TdfBlock& tdf)
    {
        FeatureDefinition f;

        std::string emptyString;

        f.world = tdf.findValue("world").get_value_or(emptyString);
        f.description = tdf.findValue("description").get_value_or(emptyString);
        f.category = tdf.findValue("category").get_value_or(emptyString);

        f.footprintX = tdf.extractUint("footprintx").get_value_or(1);
        f.footprintZ = tdf.extractUint("footprintz").get_value_or(1);
        f.height = tdf.extractUint("height").get_value_or(0);

        f.animating = tdf.extractBool("animating").get_value_or(false);
        f.fileName = tdf.findValue("filename").get_value_or(emptyString);
        f.seqName = tdf.findValue("seqname").get_value_or(emptyString);
        f.animTrans = tdf.extractBool("animtrans").get_value_or(false);
        f.seqNameShad = tdf.findValue("seqnameshad").get_value_or(emptyString);
        f.shadTrans = tdf.extractBool("shadtrans").get_value_or(false);

        f.object = tdf.findValue("object").get_value_or(emptyString);

        f.reclaimable = tdf.extractBool("reclaimable").get_value_or(false);
        f.autoreclaimable = tdf.extractBool("autoreclaimable").get_value_or(true);
        f.seqNameReclamate = tdf.findValue("seqnamereclamate").get_value_or(emptyString);
        f.featureReclamate = tdf.findValue("featurereclamate").get_value_or(emptyString);
        f.metal = tdf.extractUint("metal").get_value_or(0);
        f.energy = tdf.extractUint("energy").get_value_or(0);

        f.flamable = tdf.extractBool("flamable").get_value_or(false);
        f.seqNameBurn = tdf.findValue("seqnameburn").get_value_or(emptyString);
        f.seqNameBurnShad = tdf.findValue("seqnameburnshad").get_value_or(emptyString);
        f.featureBurnt = tdf.findValue("featureburnt").get_value_or(emptyString);
        f.burnMin = tdf.extractUint("burnmin").get_value_or(0);
        f.burnMax = tdf.extractUint("burnmax").get_value_or(0);
        f.sparkTime = tdf.extractUint("sparktime").get_value_or(0);
        f.spreadChance = tdf.extractUint("spreadchance").get_value_or(0);
        f.burnWeapon = tdf.findValue("burnweapon").get_value_or(emptyString);

        f.geothermal = tdf.extractBool("geothermal").get_value_or(false);

        f.hitDensity = tdf.extractUint("hitdensity").get_value_or(0);

        f.reproduce = tdf.extractBool("reproduce").get_value_or(false);
        f.reproduceArea = tdf.extractUint("reproducearea").get_value_or(0);

        f.noDisplayInfo = tdf.extractBool("nodisplayinfo").get_value_or(false);

        f.permanent = tdf.extractBool("permanent").get_value_or(false);

        f.blocking = tdf.extractBool("blocking").get_value_or(true);

        f.indestructible = tdf.extractBool("indestructible").get_value_or(false);
        f.damage = tdf.extractUint("damage").get_value_or(1);
        f.seqNameDie = tdf.findValue("seqnamedie").get_value_or(emptyString);
        f.featureDead = tdf.findValue("featuredead").get_value_or(emptyString);

        return f;
    }
}
