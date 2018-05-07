#ifndef RWE_COBVALUEID_H
#define RWE_COBVALUEID_H

namespace rwe
{
    enum class CobValueId
    {
        Activation = 1,
        StandingMoveOrders,
        StandingFireOrders,
        Health,
        InBuildStance,
        Busy,
        PieceXZ,
        PieceY,
        UnitXZ,
        UnitY,
        UnitHeight,
        XZAtan,
        XZHypot,
        Atan,
        Hypot,
        GroundHeight,
        BuildPercentLeft,
        YardOpen,
        BuggerOff,
        Armored,

        VeteranLevel = 32,

        UnitIsOnThisComp = 68,
        MinId,
        MaxId,
        MyId,
        UnitTeam,
        UnitBuildPercentLeft,
        UnitAllied
    };
}

#endif
