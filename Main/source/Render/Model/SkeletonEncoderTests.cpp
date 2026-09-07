#include "stdafx.h"
#include "SkeletonEncoder.h"

#include <cmath>

namespace SkeletonEncoderTests
{
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

        // 90 degree Z rotation + translation, using the same affine memory
        // convention consumed by the existing BMD transform helpers.
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

        // Uniform scale is valid for QuaternionPositionScale.
        const float scaled[] =
        {
            0,-2,0, 2,
            2, 0,0, 4,
            0, 0,2,-3
        };

        b = SkeletonEncoder::ValidateQuaternionEncoding(scaled, positions, normals, 3);
        if (!b.Success)
            return false;

        // Non-uniform scale must not silently enter the current shader format.
        const float nonUniform[] =
        {
            1,0,0,0,
            0,2,0,0,
            0,0,1,0
        };

        std::vector<float> encoded;
        if (SkeletonEncoder::EncodeQuaternionPositionScale(nonUniform, 1, encoded))
            return false;

        return true;
    }
}
