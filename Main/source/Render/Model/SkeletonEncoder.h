#pragma once

#include <cstdint>
#include <vector>

class SkeletonEncoder
{
public:
    struct ValidationResult
    {
        bool Success = false;
        float MaxPositionError = 0.0f;
        float MaxNormalError = 0.0f;
    };

    // MU CPU affine matrix convention (Main/source/Math/ZzzMathLib.cpp):
    // out.x = x*m[0][0] + y*m[0][1] + z*m[0][2] + m[0][3]
    // out.y = x*m[1][0] + y*m[1][1] + z*m[1][2] + m[1][3]
    // out.z = x*m[2][0] + y*m[2][1] + z*m[2][2] + m[2][3]
    // The source is supplied as 12 row-major floats. Matrix4x4 encoding emits
    // the transposed affine representation expected by HLSL row-vector
    // mul(position, matrix) and, equivalently, GLSL column-vector mat4 usage.
    static void EncodeMatrix4x4(const float* affine3x4,
                                std::uint32_t matrixCount,
                                std::vector<float>& outTexels);

    // Encodes a final/global MU affine bone matrix into the shader's actual
    // two-texel Quaternion + Translation + UniformScale format:
    // texel 0 = quaternion (x, y, z, w)
    // texel 1 = translation (x, y, z) + signed uniform scale (w)
    // Returns false for degenerate matrices, non-uniform scale, or shear.
    static bool EncodeQuaternionPositionScale(const float* affine3x4,
                                              std::uint32_t matrixCount,
                                              std::vector<float>& outTexels,
                                              float epsilon = 1e-4f);

    // CPU validation compares the existing BMD VectorTransform/VectorRotate
    // semantics against shader-equivalent reconstruction. This deliberately
    // uses independent memory conventions so transpose errors are detectable.
    static ValidationResult ValidateMatrixEncoding(const float* affine3x4,
                                                   const float* positions3,
                                                   const float* normals3,
                                                   std::uint32_t sampleCount,
                                                   float epsilon = 1e-4f);

    static ValidationResult ValidateQuaternionEncoding(const float* affine3x4,
                                                       const float* positions3,
                                                       const float* normals3,
                                                       std::uint32_t sampleCount,
                                                       float epsilon = 1e-4f);
};