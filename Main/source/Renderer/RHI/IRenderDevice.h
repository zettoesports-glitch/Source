#pragma once

#include "Renderer/Core/RenderTypes.h"
#include "Renderer/Core/RenderConfig.h"
#include "Renderer/Core/RenderStats.h"

namespace MU::Render
{
    class IRenderDevice
    {
    public:
        virtual ~IRenderDevice() = default;

        virtual bool Initialize(const RenderConfig& config) = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Resize(int width, int height) = 0;
        virtual void Shutdown() = 0;

        virtual RendererBackend GetBackend() const = 0;
        virtual const RenderStats& GetStats() const = 0;
        virtual void ResetStats() = 0;
    };
}
