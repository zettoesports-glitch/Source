#pragma once

#include "../Core/RenderTypes.h"
#include "../Core/RenderStats.h"

namespace Render
{
    class IRenderDevice
    {
    public:
        virtual ~IRenderDevice() {}

        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual bool IsInitialized() const = 0;

        virtual BufferHandle CreateBuffer(const BufferDesc& desc, const void* initialData) = 0;
        virtual void DestroyBuffer(BufferHandle handle) = 0;
        virtual bool UpdateBuffer(BufferHandle handle, size_t offset, size_t size, const void* data) = 0;

        virtual void BindVertexBuffer(BufferHandle handle) = 0;
        virtual void BindIndexBuffer(BufferHandle handle, IndexType type) = 0;
        virtual void BindTexture(uint32_t slot, TextureHandle handle) = 0;
        virtual void BindShader(ShaderHandle handle) = 0;
        virtual void BindPipeline(PipelineHandle handle) = 0;

        virtual void Draw(const DrawDesc& desc) = 0;
        virtual void DrawIndexed(const DrawIndexedDesc& desc) = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual const RenderStats& GetStats() const = 0;
    };
}
