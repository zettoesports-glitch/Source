#pragma once

namespace Render
{
    static const int MaxGPUSkinBones = 200;
    static const int MaxGPUSkinInfluences = 4;

    struct BoneMatrix3x4
    {
        float m[12];
    };

    struct BoneMatrix4x4
    {
        float m[16];
    };

    struct BonePalette
    {
        BoneMatrix3x4 bones[MaxGPUSkinBones];
        int count;

        BonePalette() : count(0) {}
    };

    inline void ResetBonePalette(BonePalette& palette)
    {
        palette.count = 0;
    }

    inline bool AppendBoneMatrix(BonePalette& palette, const float matrix[3][4])
    {
        if (palette.count >= MaxGPUSkinBones)
            return false;

        BoneMatrix3x4& bone = palette.bones[palette.count++];
        for (int row = 0; row < 3; ++row)
            for (int column = 0; column < 4; ++column)
                bone.m[row * 4 + column] = matrix[row][column];

        return true;
    }

    inline bool ConvertBoneMatrixTo4x4(const BoneMatrix3x4& source, BoneMatrix4x4& target)
    {
        for (int row = 0; row < 3; ++row)
            for (int column = 0; column < 4; ++column)
                target.m[row * 4 + column] = source.m[row * 4 + column];

        target.m[12] = 0.0f;
        target.m[13] = 0.0f;
        target.m[14] = 0.0f;
        target.m[15] = 1.0f;
        return true;
    }

    inline void ConvertBonePaletteTo4x4(const BonePalette& source,
                                        BoneMatrix4x4* target,
                                        int targetCount)
    {
        if (target == 0 || targetCount <= 0)
            return;

        int count = source.count;
        if (count > targetCount)
            count = targetCount;
        if (count > MaxGPUSkinBones)
            count = MaxGPUSkinBones;

        for (int i = 0; i < count; ++i)
            ConvertBoneMatrixTo4x4(source.bones[i], target[i]);
    }

    void BuildBonePaletteFromMatrices(BonePalette& palette,
                                      const float matrices[][3][4],
                                      int count);
}
