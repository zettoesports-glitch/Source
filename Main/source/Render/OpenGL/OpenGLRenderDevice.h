#pragma once

#include <unordered_map>
#include "../RHI/IRenderDevice.h"
#include "../State/BindState.h"

namespace Render
{
    class OpenGLRenderDevice : public IRenderDevice
    {
    public:
        OpenGLRenderDevice();
        virtual ~OpenGLRenderDevice();

        virtual bool Initialize();
        virtual void Shutdown();
        virtual bool IsInitialized() const;

        virtual BufferHandle CreateBuffer(const BufferDesc& desc, const void* initialData);
        virtual void DestroyBuffer(BufferHandle handle);
        virtual bool UpdateBuffer(BufferHandle handle, size_t offset, size_t size, const void* data);

        virtual void BindVertexBuffer(BufferHandle handle);
        virtual void BindIndexBuffer(BufferHandle handle, IndexType type);
        virtual void BindTexture(uint32_t slot, TextureHandle handle);
        virtual void BindShader(ShaderHandle handle);
        virtual void BindPipeline(PipelineHandle handle);

        virtual void Draw(const DrawDesc& desc);
        virtual void DrawIndexed(const DrawIndexedDesc& desc);

        virtual void BeginFrame();
        virtual void EndFrame();
        virtual const RenderStats& GetStats() const;

    private:
        struct BufferRecord
        {
            uint32_t glId;
            BufferDesc desc;
        };

        bool m_initialized;
        uint32_t m_nextBufferHandle;
        std::unordered_map<BufferHandle, BufferRecord> m_buffers;
        BindState m_state;
        RenderStats m_stats;
    };
}
