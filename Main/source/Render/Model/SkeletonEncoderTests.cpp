#include "stdafx.h"
#include "SkeletonEncoder.h"

#include <cmath>
#include <vector>

namespace SkeletonEncoderTests
{
    namespace
    {
        bool NearlyEqual(float a, float b, float epsilon = 1e-4f)
        {
            return std::fabs(a - b) <= epsilon;
        }
    }

    bool Run()
    {
        const float positions[] =
        {
            0.0f, 0.0f, 0.0f,
            1.0f, 2.0f, -1.0f,
            -3.0f, 0.5f, 4.0f
        };

        const float normals[] =
        {
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
        };

        // Identity + translation.
        const float identity[] =
        {
            1,0,0, 10,
            0,1,0,-5,
            0,0,1,  3
        };

        auto a = SkeletonEncoder::ValidateMatrixEncoding(identity, positions, normals, 3);
        auto b = SkeletonEncoder::ValidateQuaternionEncoding(identity, positions, normals, 3);
        if (!a.Success || !b.Success)
            return false;

        // 90 degree Z rotation + translation using the exact row-major affine
        // convention consumed by ZzzMathLib::VectorTransform.
        const float rotateZ[] =
        {
            0,-1,0, 2,
            1, 0,0, 4,
            0, 0,1,-3
        };

        a = SkeletonEncoder::ValidateMatrixEncoding(rotateZ, positions, normals, 3);
        b = SkeletonEncoder::ValidateQuaternionEncoding(rotateZ, positions, normals, 3);
        if (!a.Success || !b.Success)
            return false;

        // The matrix encoding must be the transpose required by HLSL
        // mul(rowVector, matrix) / generated GLSL matrix * columnVector.
        std::vector<float> encoded;
        SkeletonEncoder::EncodeMatrix4x4(rotateZ, 1, encoded);
        const float expectedMatrixTexels[] =
        {
             0, 1,0,0,
            -1, 0,0,0,
             0, 0,1,0,
             2, 4,-3,1
        };
        if (encoded.size() != 16u)
            return false;
        for (size_t i = 0; i < 16u; ++i)
        {
            if (!NearlyEqual(encoded[i], expectedMatrixTexels[i]))
                return false;
        }

        // The QPS path must keep the MU quaternion order x,y,z,w. For +90 deg
        // around Z, q is equivalent to (0,0,+sqrt(1/2),+sqrt(1/2)).
        encoded.clear();
        if (!SkeletonEncoder::EncodeQuaternionPositionScale(rotateZ, 1, encoded))
            return false;
        const float halfSqrt = 0.70710678f;
        if (encoded.size() != 8u ||
            !NearlyEqual(encoded[0], 0.0f) ||
            !NearlyEqual(encoded[1], 0.0f) ||
            !NearlyEqual(encoded[2], halfSqrt) ||
            !NearlyEqual(encoded[3], halfSqrt) ||
            !NearlyEqual(encoded[4], 2.0f) ||
            !NearlyEqual(encoded[5], 4.0f) ||
            !NearlyEqual(encoded[6], -3.0f) ||
            !NearlyEqual(encoded[7], 1.0f))
        {
            return false;
        }

        // Uniform positive scale is valid for QuaternionPositionScale.
        const float scaled[] =
        {
            0,-2,0, 2,
            2, 0,0, 4,
            0, 0,2,-3
        };

        b = SkeletonEncoder::ValidateQuaternionEncoding(scaled, positions, normals, 3);
        if (!b.Success)
            return false;

        // Uniform negative scale is representable by signed scale + proper
        // rotation and must reconstruct the same affine transform.
        const float negativeScaled[] =
        {
            -1, 0, 0, 1,
             0,-1, 0, 2,
             0, 0,-1, 3
        };
        b = SkeletonEncoder::ValidateQuaternionEncoding(negativeScaled, positions, normals, 3);
        if (!b.Success)
            return false;

        encoded.clear();
        if (!SkeletonEncoder::EncodeQuaternionPositionScale(negativeScaled, 1, encoded) ||
            encoded.size() != 8u || !NearlyEqual(encoded[7], -1.0f))
        {
            return false;
        }

        // Non-uniform scale must not silently enter the current shader format.
        const float nonUniform[] =
        {
            1,0,0,0,
            0,2,0,0,
            0,0,1,0
        };

        encoded.clear();
        if (SkeletonEncoder::EncodeQuaternionPositionScale(nonUniform, 1, encoded))
            return false;

        // Shear is not representable by Quaternion + UniformScale either.
        const float shear[] =
        {
            1,0.25f,0,0,
            0,1,0,0,
            0,0,1,0
        };
        encoded.clear();
        if (SkeletonEncoder::EncodeQuaternionPositionScale(shear, 1, encoded))
            return false;

        return true;
    }
}
