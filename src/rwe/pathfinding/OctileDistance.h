#ifndef RWE_OCTILEDISTANCE_H
#define RWE_OCTILEDISTANCE_H

namespace rwe
{
    struct OctileDistance
    {
        unsigned int straight;
        unsigned int diagonal;

        OctileDistance(unsigned int straight, unsigned int diagonal);

        bool operator==(const OctileDistance& rhs) const;

        bool operator!=(const OctileDistance& rhs) const;
    };
}

#endif
