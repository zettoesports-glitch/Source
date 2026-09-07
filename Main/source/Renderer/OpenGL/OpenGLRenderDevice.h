#pragma once

#include "Renderer/RHI/IRenderDevice.h"
#include "Renderer/State/BindState.h"

namespace MU::Render
{
    class OpenGLRenderDevice final : public IRenderDevice
    {
    public:
        bool Initialize(const RenderConfig& config) override;
        void BeginFrame() override;
        void EndFrame() override;
        void Resize(int width, int height) override;
        void Shutdown() override;

        RendererBackend GetBackend() const override
        {
            return RendererBackend::OpenGL;
        }

        const RenderStats& GetStats() const override
        {
            return m_Stats;
        }

        void ResetStats() override
        {
            m_Stats.Reset();
        }

        const RenderViewport& GetViewport() const
        {
            return m_Viewport;
        }

        BindState& GetBindState()
        {
            return m_BindState;
        }

        const BindState& GetBindState() const
        {
            return m_BindState;
        }

        bool IsInitialized() const
        {
            return m_Initialized;
        }

    private:
        RenderViewport m_Viewport{};
        RenderStats m_Stats{};
        BindState m_BindState{};
        bool m_Initialized = false;
    };
}
