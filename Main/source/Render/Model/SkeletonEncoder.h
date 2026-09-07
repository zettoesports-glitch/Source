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

    // CPU BMD affine matrix convention:
    // out.x = x*m[0][0] + y*m[1][0] + z*m[2][0] + m[3][0]
    // The source is supplied as 12 floats in row-major memory used by the
    // existing BMD code. Encoding transposes explicitly for GLSL column-vector
    // mat4 construction.
    static void EncodeMatrix4x4(const float* affine3x4,
                                std::uint32_t matrixCount,
                                std::vector<float>& outTexels);

    // Encodes the global affine matrix into the shader's actual two-texel
    // Quaternion + Translation + UniformScale format.
    // Returns false when scale is non-uniform or the 3x3 contains shear.
    static bool EncodeQuaternionPositionScale(const float* affine3x4,
                                              std::uint32_t matrixCount,
                                              std::vector<float>& outTexels,
                                              float epsilon = 1e-4f);

    // CPU validation of encoded data against the BMD affine transform.
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