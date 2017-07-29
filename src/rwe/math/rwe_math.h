#ifndef RWE_MATH_RWE_MATH_H
#define RWE_MATH_RWE_MATH_H

namespace rwe
{
    /**
     * Quantizes a value to the nearest multiple of the given interval.
     *
     * @param value The value to quantize.
     * @param interval The interval. Must be a positive (>0) number.
     * @return The quantized value.
     */
    float snapToInterval(float value, float interval);

    /**
     * Quantizes a value to the multiple of the given interval that is closest to zero.
     *
     * @param value The value to quantize.
     * @param interval The interval. Must be a positive (>0) number.
     * @return The quantized value.
     */
    float truncateToInterval(float value, float interval);

    /**
     * Returns true if a and b both have the same sign.
     */
    bool sameSign(float a, float b);
}

#endif
