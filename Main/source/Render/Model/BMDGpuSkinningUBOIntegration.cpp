#include "../../stdafx.h"
#include "../../New_ModelBMD.h"
#include "GPUSkinning.h"

#if jdk_shader_local330

namespace OGL330MODEL
{
    bool CGMMeshShader::UploadBonePaletteUBO(const std::vector<float>& palette)
    {
        if (palette.empty())
            return false;

        Render::BonePalette gpuPalette;
        const size_t availableBones = palette.size() / 12u;
        const size_t boneCount = availableBones > static_cast<size_t>(Render::MaxGPUSkinBones)
            ? static_cast<size_t>(Render::MaxGPUSkinBones)
            : availableBones;

        for (size_t bone = 0; bone < boneCount; ++bone)
        {
            Render::BoneMatrix3x4& target = gpuPalette.bones[gpuPalette.count++];
            const float* source = &palette[bone * 12u];

            for (int value = 0; value < 12; ++value)
                target.m[value] = source[value];
        }

        return m_BoneUBO.Upload(gpuPalette);
    }
}

#endif
