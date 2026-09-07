#pragma once

#include "BMDGpuSkinningUBOBinding.h"

namespace Render
{
    // Transition bridge for the existing BMD renderer.
    // The legacy uniform path remains authoritative until the runtime shader
    // and VAO are migrated to the Bone UBO contract.
    class BMDGpuSkinningRuntimeBridge
    {
    public:
        BMDGpuSkinningRuntimeBridge() : m_enabled(false) {}

        void SetEnabled(bool enabled) { m_enabled = enabled; }
        bool IsEnabled() const { return m_enabled; }

        bool Upload(const BonePalette& palette)
        {
            if (!m_enabled)
                return false;
            return m_ubo.Upload(palette);
        }

        void Bind() const
        {
            if (m_enabled)
                m_ubo.Bind();
        }

        void Release()
        {
            m_ubo.Release();
        }

    private:
        BMDGpuSkinningUBOBinding m_ubo;
        bool m_enabled;
    };
}
