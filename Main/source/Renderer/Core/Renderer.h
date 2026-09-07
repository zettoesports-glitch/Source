#pragma once

#include "Renderer/Core/RenderConfig.h"
#include "Renderer/RHI/IRenderDevice.h"

namespace MU::Render
{
    class Renderer
    {
    public:
        explicit Renderer(IRenderDevice& device)
            : m_Device(device)
        {
        }

        bool Initialize(const RenderConfig& config)
        {
            return m_Device.Initialize(config);
        }

        void BeginFrame()
        {
            m_Device.BeginFrame();
        }

        void EndFrame()
        {
            m_Device.EndFrame();
        }

        void Resize(int width, int height)
        {
            m_Device.Resize(width, height);
        }

        void Shutdown()
        {
            m_Device.Shutdown();
        }

        IRenderDevice& GetDevice()
        {
            return m_Device;
        }

        const IRenderDevice& GetDevice() const
        {
            return m_Device;
        }

    private:
        IRenderDevice& m_Device;
    };
}
