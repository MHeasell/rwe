#pragma once

namespace rwe
{
    /** A distance in grid squares separated into straight and diagonal components. */
    struct OctileDistance
    {
        int straight{0};
        int diagonal{0};

        static OctileDistance fromXAndY(int x, int y);

        OctileDistance() = default;
        OctileDistance(int straight, int diagonal);

        bool operator==(const OctileDistance& rhs) const;

        bool operator!=(const OctileDistance& rhs) const;

        bool operator<(const OctileDistance& rhs) const;

        bool operator>(const OctileDistance& rhs) const;

        bool operator<=(const OctileDistance& rhs) const;

        bool operator>=(const OctileDistance& rhs) const;

        OctileDistance operator+(const OctileDistance& rhs) const;

        /**
         * Returns the distance as a float.
         * Assumes that the length of the side of a square is 1,
         * and that we are dealing with regular old Euclidian space.
         */
        float asFloat() const;
    };
}
