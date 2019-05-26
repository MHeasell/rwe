#include "Matrix4f.h"
#include <cmath>

namespace rwe
{
    Matrix4f Matrix4f::identity()
    {
        Matrix4f m;

        m.data[0] = 1.0f;
        m.data[1] = 0.0f;
        m.data[2] = 0.0f;
        m.data[3] = 0.0f;

        m.data[4] = 0.0f;
        m.data[5] = 1.0f;
        m.data[6] = 0.0f;
        m.data[7] = 0.0f;

        m.data[8] = 0.0f;
        m.data[9] = 0.0f;
        m.data[10] = 1.0f;
        m.data[11] = 0.0f;

        m.data[12] = 0.0f;
        m.data[13] = 0.0f;
        m.data[14] = 0.0f;
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::translation(const Vector3f& v)
    {
        Matrix4f m;

        m.data[0] = 1.0f;
        m.data[1] = 0.0f;
        m.data[2] = 0.0f;
        m.data[3] = 0.0f;

        m.data[4] = 0.0f;
        m.data[5] = 1.0f;
        m.data[6] = 0.0f;
        m.data[7] = 0.0f;

        m.data[8] = 0.0f;
        m.data[9] = 0.0f;
        m.data[10] = 1.0f;
        m.data[11] = 0.0f;

        m.data[12] = v.x;
        m.data[13] = v.y;
        m.data[14] = v.z;
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::scale(float size)
    {
        Matrix4f m;

        m.data[0] = size;
        m.data[1] = 0.0f;
        m.data[2] = 0.0f;
        m.data[3] = 0.0f;

        m.data[4] = 0.0f;
        m.data[5] = size;
        m.data[6] = 0.0f;
        m.data[7] = 0.0f;

        m.data[8] = 0.0f;
        m.data[9] = 0.0f;
        m.data[10] = size;
        m.data[11] = 0.0f;

        m.data[12] = 0.0f;
        m.data[13] = 0.0f;
        m.data[14] = 0.0f;
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::scale(const Vector3f& v)
    {
        Matrix4f m;

        m.data[0] = v.x;
        m.data[1] = 0.0f;
        m.data[2] = 0.0f;
        m.data[3] = 0.0f;

        m.data[4] = 0.0f;
        m.data[5] = v.y;
        m.data[6] = 0.0f;
        m.data[7] = 0.0f;

        m.data[8] = 0.0f;
        m.data[9] = 0.0f;
        m.data[10] = v.z;
        m.data[11] = 0.0f;

        m.data[12] = 0.0f;
        m.data[13] = 0.0f;
        m.data[14] = 0.0f;
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::shearXZ(float x, float z)
    {
        Matrix4f m;

        m.data[0] = 1.0f;
        m.data[1] = 0.0f;
        m.data[2] = 0.0f;
        m.data[3] = 0.0f;

        m.data[4] = x;
        m.data[5] = 1.0f;
        m.data[6] = z;
        m.data[7] = 0.0f;

        m.data[8] = 0.0f;
        m.data[9] = 0.0f;
        m.data[10] = 1.0f;
        m.data[11] = 0.0f;

        m.data[12] = 0.0f;
        m.data[13] = 0.0f;
        m.data[14] = 0.0f;
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::cabinetProjection(float x, float y)
    {
        Matrix4f m;

        m.data[0] = 1.0f;
        m.data[1] = 0.0f;
        m.data[2] = 0.0f;
        m.data[3] = 0.0f;

        m.data[4] = 0.0f;
        m.data[5] = 1.0f;
        m.data[6] = 0.0f;
        m.data[7] = 0.0f;

        m.data[8] = x;
        m.data[9] = y;
        m.data[10] = 1.0f;
        m.data[11] = 0.0f;

        m.data[12] = 0.0f;
        m.data[13] = 0.0f;
        m.data[14] = 0.0f;
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::orthographicProjection(float left, float right, float bottom, float top, float nearVal, float farVal)
    {
        if (left == right || top == bottom || nearVal == farVal)
        {
            throw std::logic_error("Invalid orthographic matrix parameters");
        }

        Matrix4f m;

        m.data[0] = 2.0f / (right - left);
        m.data[1] = 0.0f;
        m.data[2] = 0.0f;
        m.data[3] = 0.0f;

        m.data[4] = 0.0f;
        m.data[5] = 2.0f / (top - bottom);
        m.data[6] = 0.0f;
        m.data[7] = 0.0f;

        m.data[8] = 0.0f;
        m.data[9] = 0.0f;
        m.data[10] = -2.0f / (farVal - nearVal);
        m.data[11] = 0.0f;

        m.data[12] = -((right + left) / (right - left));
        m.data[13] = -((top + bottom) / (top - bottom));
        m.data[14] = -((farVal + nearVal) / (farVal - nearVal));
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::inverseOrthographicProjection(float left, float right, float bottom, float top, float nearVal, float farVal)
    {
        if (left == right || top == bottom || nearVal == farVal)
        {
            throw std::logic_error("Invalid orthographic matrix parameters");
        }

        Matrix4f m;

        m.data[0] = (right - left) / 2.0f;
        m.data[1] = 0.0f;
        m.data[2] = 0.0f;
        m.data[3] = 0.0f;

        m.data[4] = 0.0f;
        m.data[5] = (top - bottom) / 2.0f;
        m.data[6] = 0.0f;
        m.data[7] = 0.0f;

        m.data[8] = 0.0f;
        m.data[9] = 0.0f;
        m.data[10] = (farVal - nearVal) / -2.0f;
        m.data[11] = 0.0f;

        m.data[12] = (right + left) / 2.0f;
        m.data[13] = (top + bottom) / 2.0f;
        m.data[14] = (farVal + nearVal) / -2.0f;
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::rotationToAxes(const Vector3f& side, const Vector3f& up, const Vector3f& forward)
    {
        Matrix4f m;

        m.data[0] = side.x;
        m.data[1] = up.x;
        m.data[2] = forward.x;
        m.data[3] = 0.0f;

        m.data[4] = side.y;
        m.data[5] = up.y;
        m.data[6] = forward.y;
        m.data[7] = 0.0f;

        m.data[8] = side.z;
        m.data[9] = up.z;
        m.data[10] = forward.z;
        m.data[11] = 0.0f;

        m.data[12] = 0.0f;
        m.data[13] = 0.0f;
        m.data[14] = 0.0f;
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::transposed() const
    {
        Matrix4f m;
        for (int i = 0; i < 16; ++i)
        {
            int col = i / 4;
            int row = i % 4;
            m.data[(row * 4) + col] = data[i];
        }

        return m;
    }

    Vector3f Matrix4f::mult3x3(const Vector3f& v) const{
        Vector3f r;

        // clang-format off
        r.x = (data[0] * v.x) + (data[4] * v.y) + (data[ 8] * v.z);
        r.y = (data[1] * v.x) + (data[5] * v.y) + (data[ 9] * v.z);
        r.z = (data[2] * v.x) + (data[6] * v.y) + (data[10] * v.z);
        // clang-format on

        return r;
    }

    Matrix4f Matrix4f::rotationX(float angle)
    {
        Matrix4f m;
        float s = std::sin(angle);
        float c = std::cos(angle);

        m.data[0] = 1.0f;
        m.data[1] = 0.0f;
        m.data[2] = 0.0f;
        m.data[3] = 0.0f;

        m.data[4] = 0.0f;
        m.data[5] = c;
        m.data[6] = s;
        m.data[7] = 0.0f;

        m.data[8] = 0.0f;
        m.data[9] = -s;
        m.data[10] = c;
        m.data[11] = 0.0f;

        m.data[12] = 0.0f;
        m.data[13] = 0.0f;
        m.data[14] = 0.0f;
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::rotationY(float angle)
    {
        Matrix4f m;
        float s = std::sin(angle);
        float c = std::cos(angle);

        m.data[0] = c;
        m.data[1] = 0.0f;
        m.data[2] = -s;
        m.data[3] = 0.0f;

        m.data[4] = 0.0f;
        m.data[5] = 1.0f;
        m.data[6] = 0.0f;
        m.data[7] = 0.0f;

        m.data[8] = s;
        m.data[9] = 0.0f;
        m.data[10] = c;
        m.data[11] = 0.0f;

        m.data[12] = 0.0f;
        m.data[13] = 0.0f;
        m.data[14] = 0.0f;
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::rotationZ(float angle)
    {
        Matrix4f m;
        float s = std::sin(angle);
        float c = std::cos(angle);

        m.data[0] = c;
        m.data[1] = s;
        m.data[2] = 0.0f;
        m.data[3] = 0.0f;

        m.data[4] = -s;
        m.data[5] = c;
        m.data[6] = 0.0f;
        m.data[7] = 0.0f;

        m.data[8] = 0.0f;
        m.data[9] = 0.0f;
        m.data[10] = 1.0f;
        m.data[11] = 0.0f;

        m.data[12] = 0.0f;
        m.data[13] = 0.0f;
        m.data[14] = 0.0f;
        m.data[15] = 1.0f;

        return m;
    }

    Matrix4f Matrix4f::rotationXYZ(const Vector3f& angles)
    {
        return Matrix4f::rotationZ(angles.z) * Matrix4f::rotationY(angles.y) * Matrix4f::rotationX(angles.x);
    }

    Matrix4f Matrix4f::rotationZXY(const Vector3f& angles)
    {
        return Matrix4f::rotationY(angles.y) * Matrix4f::rotationX(angles.x) * Matrix4f::rotationZ(angles.z);
    }

    Matrix4f operator*(const Matrix4f& a, const Matrix4f& b)
    {
        Matrix4f m;

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

    Vector3f operator*(const Vector3f& a, const Matrix4f& b)
    {
        Vector3f v;

        // clang-format off
        v.x = (a.x * b.data[0]) + (a.y * b.data[1]) + (a.z * b.data[ 2]) + (b.data[ 3]);
        v.y = (a.x * b.data[4]) + (a.y * b.data[5]) + (a.z * b.data[ 6]) + (b.data[ 7]);
        v.z = (a.x * b.data[8]) + (a.y * b.data[9]) + (a.z * b.data[10]) + (b.data[11]);
        // clang-format on

        return v;
    }

    Vector3f operator*(const Matrix4f& a, const Vector3f& b)
    {
        Vector3f v;

        // clang-format off
        v.x = (a.data[0] * b.x) + (a.data[4] * b.y) + (a.data[ 8] * b.z) + (a.data[12]);
        v.y = (a.data[1] * b.x) + (a.data[5] * b.y) + (a.data[ 9] * b.z) + (a.data[13]);
        v.z = (a.data[2] * b.x) + (a.data[6] * b.y) + (a.data[10] * b.z) + (a.data[14]);
        // clang-format on

        return v;
    }
}
