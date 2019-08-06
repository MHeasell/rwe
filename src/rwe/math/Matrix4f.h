#pragma once

#include <rwe/math/Vector3f.h>

namespace rwe
{
    template <typename Val>
    struct Matrix4x
    {
        static Matrix4x identity()
        {
            Matrix4x m;

            m.data[0] = 1;
            m.data[1] = 0;
            m.data[2] = 0;
            m.data[3] = 0;

            m.data[4] = 0;
            m.data[5] = 1;
            m.data[6] = 0;
            m.data[7] = 0;

            m.data[8] = 0;
            m.data[9] = 0;
            m.data[10] = 1;
            m.data[11] = 0;

            m.data[12] = 0;
            m.data[13] = 0;
            m.data[14] = 0;
            m.data[15] = 1;

            return m;
        }

        static Matrix4x translation(const Vector3x<Val>& v)
        {
            Matrix4x m;

            m.data[0] = Val(1);
            m.data[1] = Val(0);
            m.data[2] = Val(0);
            m.data[3] = Val(0);

            m.data[4] = Val(0);
            m.data[5] = Val(1);
            m.data[6] = Val(0);
            m.data[7] = Val(0);

            m.data[8] = Val(0);
            m.data[9] = Val(0);
            m.data[10] = Val(1);
            m.data[11] = Val(0);

            m.data[12] = v.x;
            m.data[13] = v.y;
            m.data[14] = v.z;
            m.data[15] = Val(1);

            return m;
        }

        static Matrix4x scale(Val size)
        {
            Matrix4x m;

            m.data[0] = size;
            m.data[1] = 0;
            m.data[2] = 0;
            m.data[3] = 0;

            m.data[4] = 0;
            m.data[5] = size;
            m.data[6] = 0;
            m.data[7] = 0;

            m.data[8] = 0;
            m.data[9] = 0;
            m.data[10] = size;
            m.data[11] = 0;

            m.data[12] = 0;
            m.data[13] = 0;
            m.data[14] = 0;
            m.data[15] = 1;

            return m;
        }

        static Matrix4x scale(const Vector3x<Val>& v)
        {
            Matrix4x m;

            m.data[0] = v.x;
            m.data[1] = 0;
            m.data[2] = 0;
            m.data[3] = 0;

            m.data[4] = 0;
            m.data[5] = v.y;
            m.data[6] = 0;
            m.data[7] = 0;

            m.data[8] = 0;
            m.data[9] = 0;
            m.data[10] = v.z;
            m.data[11] = 0;

            m.data[12] = 0;
            m.data[13] = 0;
            m.data[14] = 0;
            m.data[15] = 1;

            return m;
        }

        static Matrix4x shearXZ(Val x, Val z)
        {
            Matrix4x m;

            m.data[0] = 1;
            m.data[1] = 0;
            m.data[2] = 0;
            m.data[3] = 0;

            m.data[4] = x;
            m.data[5] = 1;
            m.data[6] = z;
            m.data[7] = 0;

            m.data[8] = 0;
            m.data[9] = 0;
            m.data[10] = 1;
            m.data[11] = 0;

            m.data[12] = 0;
            m.data[13] = 0;
            m.data[14] = 0;
            m.data[15] = 1;

            return m;
        }

        static Matrix4x cabinetProjection(Val x, Val y)
        {
            Matrix4x m;

            m.data[0] = 1;
            m.data[1] = 0;
            m.data[2] = 0;
            m.data[3] = 0;

            m.data[4] = 0;
            m.data[5] = 1;
            m.data[6] = 0;
            m.data[7] = 0;

            m.data[8] = x;
            m.data[9] = y;
            m.data[10] = 1;
            m.data[11] = 0;

            m.data[12] = 0;
            m.data[13] = 0;
            m.data[14] = 0;
            m.data[15] = 1;

            return m;
        }

        static Matrix4x orthographicProjection(Val left, Val right, Val bottom, Val top, Val nearVal, Val farVal)
        {
            if (left == right || top == bottom || nearVal == farVal)
            {
                throw std::logic_error("Invalid orthographic matrix parameters");
            }

            Matrix4x m;

            m.data[0] = 2 / (right - left);
            m.data[1] = 0;
            m.data[2] = 0;
            m.data[3] = 0;

            m.data[4] = 0;
            m.data[5] = 2 / (top - bottom);
            m.data[6] = 0;
            m.data[7] = 0;

            m.data[8] = 0;
            m.data[9] = 0;
            m.data[10] = -2 / (farVal - nearVal);
            m.data[11] = 0;

            m.data[12] = -((right + left) / (right - left));
            m.data[13] = -((top + bottom) / (top - bottom));
            m.data[14] = -((farVal + nearVal) / (farVal - nearVal));
            m.data[15] = 1;

            return m;
        }

        static Matrix4x inverseOrthographicProjection(Val left, Val right, Val bottom, Val top, Val nearVal, Val farVal)
        {
            if (left == right || top == bottom || nearVal == farVal)
            {
                throw std::logic_error("Invalid orthographic matrix parameters");
            }

            Matrix4x m;

            m.data[0] = (right - left) / 2;
            m.data[1] = 0;
            m.data[2] = 0;
            m.data[3] = 0;

            m.data[4] = 0;
            m.data[5] = (top - bottom) / 2;
            m.data[6] = 0;
            m.data[7] = 0;

            m.data[8] = 0;
            m.data[9] = 0;
            m.data[10] = (farVal - nearVal) / -2;
            m.data[11] = 0;

            m.data[12] = (right + left) / 2;
            m.data[13] = (top + bottom) / 2;
            m.data[14] = (farVal + nearVal) / -2;
            m.data[15] = 1;

            return m;
        }

        static Matrix4x rotationToAxes(const Vector3x<Val>& side, const Vector3x<Val>& up, const Vector3x<Val>& forward)
        {
            Matrix4x m;

            m.data[0] = side.x;
            m.data[1] = up.x;
            m.data[2] = forward.x;
            m.data[3] = 0;

            m.data[4] = side.y;
            m.data[5] = up.y;
            m.data[6] = forward.y;
            m.data[7] = 0;

            m.data[8] = side.z;
            m.data[9] = up.z;
            m.data[10] = forward.z;
            m.data[11] = 0;

            m.data[12] = 0;
            m.data[13] = 0;
            m.data[14] = 0;
            m.data[15] = 1;

            return m;
        }


        /** Anti-clockwise rotation about the X axis. */
        static Matrix4x rotationX(Val angle)
        {
            Matrix4x m;
            auto s = std::sin(angle);
            auto c = std::cos(angle);

            m.data[0] = 1;
            m.data[1] = 0;
            m.data[2] = 0;
            m.data[3] = 0;

            m.data[4] = 0;
            m.data[5] = c;
            m.data[6] = s;
            m.data[7] = 0;

            m.data[8] = 0;
            m.data[9] = -s;
            m.data[10] = c;
            m.data[11] = 0;

            m.data[12] = 0;
            m.data[13] = 0;
            m.data[14] = 0;
            m.data[15] = 1;

            return m;
        }

        /** Anti-clockwise rotation about the Y axis. */
        static Matrix4x rotationY(Val angle)
        {
            return rotationY(std::sin(angle), std::cos(angle));
        }

        /** Anti-clockwise rotation about the Y axis. */
        static Matrix4x rotationY(Val sinAngle, Val cosAngle)
        {
            Matrix4x m;
            auto s = sinAngle;
            auto c = cosAngle;

            m.data[0] = c;
            m.data[1] = Val(0);
            m.data[2] = -s;
            m.data[3] = Val(0);

            m.data[4] = Val(0);
            m.data[5] = Val(1);
            m.data[6] = Val(0);
            m.data[7] = Val(0);

            m.data[8] = s;
            m.data[9] = Val(0);
            m.data[10] = c;
            m.data[11] = Val(0);

            m.data[12] = Val(0);
            m.data[13] = Val(0);
            m.data[14] = Val(0);
            m.data[15] = Val(1);

            return m;
        }

        /** Anti-clockwise rotation about the Z axis. */
        static Matrix4x rotationZ(Val angle)
        {
            Matrix4x m;
            auto s = std::sin(angle);
            auto c = std::cos(angle);

            m.data[0] = c;
            m.data[1] = s;
            m.data[2] = 0;
            m.data[3] = 0;

            m.data[4] = -s;
            m.data[5] = c;
            m.data[6] = 0;
            m.data[7] = 0;

            m.data[8] = 0;
            m.data[9] = 0;
            m.data[10] = 1;
            m.data[11] = 0;

            m.data[12] = 0;
            m.data[13] = 0;
            m.data[14] = 0;
            m.data[15] = 1;

            return m;
        }

        static Matrix4x rotationXYZ(const Vector3x<Val>& angles)
        {
            return rotationZ(angles.z) * rotationY(angles.y) * rotationX(angles.x);
        }

        static Matrix4x rotationZXY(const Vector3x<Val>& angles)
        {
            return rotationY(angles.y) * rotationX(angles.x) * rotationZ(angles.z);
        }

        /**
         * Elements are stored in column-major order,
         * i.e. the array is indexed E[(column * column_length) + row] or E[(x * height) + y].
         */
        Val data[16];

        Matrix4x transposed() const
        {
            Matrix4x m;
            for (int i = 0; i < 16; ++i)
            {
                int col = i / 4;
                int row = i % 4;
                m.data[(row * 4) + col] = data[i];
            }

            return m;
        }

        /**
         * Multiplies the upper-left 3x3 portion of this matrix by the given vector.
         */
        Vector3x<Val> mult3x3(const Vector3x<Val>& v) const
        {
            Vector3x<Val> r;

            // clang-format off
            r.x = (data[0] * v.x) + (data[4] * v.y) + (data[ 8] * v.z);
            r.y = (data[1] * v.x) + (data[5] * v.y) + (data[ 9] * v.z);
            r.z = (data[2] * v.x) + (data[6] * v.y) + (data[10] * v.z);
            // clang-format on

            return r;
        }
    };

    template <typename Val>
    Matrix4x<Val> operator*(const Matrix4x<Val>& a, const Matrix4x<Val>& b)
    {
        Matrix4x<Val> m;

        // clang-format off
        m.data[ 0] = (a.data[0] * b.data[ 0]) + (a.data[4] * b.data[ 1]) + (a.data[ 8] * b.data[ 2]) + (a.data[12] * b.data[ 3]);
        m.data[ 1] = (a.data[1] * b.data[ 0]) + (a.data[5] * b.data[ 1]) + (a.data[ 9] * b.data[ 2]) + (a.data[13] * b.data[ 3]);
        m.data[ 2] = (a.data[2] * b.data[ 0]) + (a.data[6] * b.data[ 1]) + (a.data[10] * b.data[ 2]) + (a.data[14] * b.data[ 3]);
        m.data[ 3] = (a.data[3] * b.data[ 0]) + (a.data[7] * b.data[ 1]) + (a.data[11] * b.data[ 2]) + (a.data[15] * b.data[ 3]);

        m.data[ 4] = (a.data[0] * b.data[ 4]) + (a.data[4] * b.data[ 5]) + (a.data[ 8] * b.data[ 6]) + (a.data[12] * b.data[ 7]);
        m.data[ 5] = (a.data[1] * b.data[ 4]) + (a.data[5] * b.data[ 5]) + (a.data[ 9] * b.data[ 6]) + (a.data[13] * b.data[ 7]);
        m.data[ 6] = (a.data[2] * b.data[ 4]) + (a.data[6] * b.data[ 5]) + (a.data[10] * b.data[ 6]) + (a.data[14] * b.data[ 7]);
        m.data[ 7] = (a.data[3] * b.data[ 4]) + (a.data[7] * b.data[ 5]) + (a.data[11] * b.data[ 6]) + (a.data[15] * b.data[ 7]);

        m.data[ 8] = (a.data[0] * b.data[ 8]) + (a.data[4] * b.data[ 9]) + (a.data[ 8] * b.data[10]) + (a.data[12] * b.data[11]);
        m.data[ 9] = (a.data[1] * b.data[ 8]) + (a.data[5] * b.data[ 9]) + (a.data[ 9] * b.data[10]) + (a.data[13] * b.data[11]);
        m.data[10] = (a.data[2] * b.data[ 8]) + (a.data[6] * b.data[ 9]) + (a.data[10] * b.data[10]) + (a.data[14] * b.data[11]);
        m.data[11] = (a.data[3] * b.data[ 8]) + (a.data[7] * b.data[ 9]) + (a.data[11] * b.data[10]) + (a.data[15] * b.data[11]);

        m.data[12] = (a.data[0] * b.data[12]) + (a.data[4] * b.data[13]) + (a.data[ 8] * b.data[14]) + (a.data[12] * b.data[15]);
        m.data[13] = (a.data[1] * b.data[12]) + (a.data[5] * b.data[13]) + (a.data[ 9] * b.data[14]) + (a.data[13] * b.data[15]);
        m.data[14] = (a.data[2] * b.data[12]) + (a.data[6] * b.data[13]) + (a.data[10] * b.data[14]) + (a.data[14] * b.data[15]);
        m.data[15] = (a.data[3] * b.data[12]) + (a.data[7] * b.data[13]) + (a.data[11] * b.data[14]) + (a.data[15] * b.data[15]);
        // clang-format on

        return m;
    }

    /**
     * Multiplication of row vector a by matrix b.
     * It is implied that the vector contains a fourth component,
     * w, which is 1.
     * The last column of the matrix is ignored.
     */
    template <typename Val>
    Vector3x<Val> operator*(const Vector3x<Val>& a, const Matrix4x<Val>& b)
    {
        Vector3x<Val> v;

        // clang-format off
        v.x = (a.x * b.data[0]) + (a.y * b.data[1]) + (a.z * b.data[ 2]) + (b.data[ 3]);
        v.y = (a.x * b.data[4]) + (a.y * b.data[5]) + (a.z * b.data[ 6]) + (b.data[ 7]);
        v.z = (a.x * b.data[8]) + (a.y * b.data[9]) + (a.z * b.data[10]) + (b.data[11]);
        // clang-format on

        return v;
    }

    /**
     * Multiplication of matrix a by column vector b.
     * It is implied that the vector contains a fourth component,
     * w, which is 1.
     * The last row of the matrix is ignored.
     */
    template <typename Val>
    Vector3x<Val> operator*(const Matrix4x<Val>& a, const Vector3x<Val>& b)
    {
        Vector3x<Val> v;

        // clang-format off
        v.x = (a.data[0] * b.x) + (a.data[4] * b.y) + (a.data[ 8] * b.z) + (a.data[12]);
        v.y = (a.data[1] * b.x) + (a.data[5] * b.y) + (a.data[ 9] * b.z) + (a.data[13]);
        v.z = (a.data[2] * b.x) + (a.data[6] * b.y) + (a.data[10] * b.z) + (a.data[14]);
        // clang-format on

        return v;
    }

    using Matrix4f = Matrix4x<float>;
}
