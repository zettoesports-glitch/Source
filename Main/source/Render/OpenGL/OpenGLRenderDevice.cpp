#include "stdafx.h"
#include "OpenGLRenderDevice.h"

#include "../Core/RenderConfig.h"
#include "../Uniforms/BoneUBO.h"

namespace Render
{
    OpenGLRenderDevice::OpenGLRenderDevice()
        : m_initialized(false), m_nextBufferHandle(1) {}

    OpenGLRenderDevice::~OpenGLRenderDevice()
    {
        Shutdown();
    }

    bool OpenGLRenderDevice::Initialize()
    {
        if (m_initialized)
            return true;
        m_state.Reset();
        m_stats.Reset();
        m_initialized = true;
        return true;
    }

    void OpenGLRenderDevice::Shutdown()
    {
        m_buffers.clear();
        m_state.Reset();
        m_initialized = false;
    }

    bool OpenGLRenderDevice::IsInitialized() const { return m_initialized; }

    BufferHandle OpenGLRenderDevice::CreateBuffer(const BufferDesc& desc, const void* initialData)
    {
        if (!m_initialized || desc.size == 0)
            return InvalidBuffer;

        // Allocation is intentionally kept behind this RHI boundary. The first
        // integration step can use the existing CoreGLCompat streaming VBO;
        // no legacy call site is changed by this foundation layer.
        BufferHandle handle = m_nextBufferHandle++;
        BufferRecord record;
        record.glId = 0;
        record.desc = desc;
        m_buffers[handle] = record;
        if (initialData)
        {
            m_stats.bufferUploads++;
            m_stats.bufferUploadBytes += desc.size;
        }
        return handle;
    }

    void OpenGLRenderDevice::DestroyBuffer(BufferHandle handle)
    {
        if (handle != InvalidBuffer)
            m_buffers.erase(handle);
        if (m_state.GetVertexBuffer() == handle)
            m_state.SetVertexBuffer(InvalidBuffer);
        if (m_state.GetIndexBuffer() == handle)
            m_state.SetIndexBuffer(InvalidBuffer);
    }

    bool OpenGLRenderDevice::UpdateBuffer(BufferHandle handle, size_t offset, size_t size, const void* data)
    {
        std::unordered_map<BufferHandle, BufferRecord>::iterator it = m_buffers.find(handle);
        if (!m_initialized || it == m_buffers.end() || data == NULL || size == 0 || offset > it->second.desc.size || size > it->second.desc.size - offset)
            return false;
        m_stats.bufferUploads++;
        m_stats.bufferUploadBytes += size;
        return true;
    }

    void OpenGLRenderDevice::BindVertexBuffer(BufferHandle handle)
    {
        if (m_state.SetVertexBuffer(handle)) {}
    }

    void OpenGLRenderDevice::BindIndexBuffer(BufferHandle handle, IndexType type)
    {
        m_state.SetIndexBuffer(handle);
        m_state.SetIndexType(type);
    }

    void OpenGLRenderDevice::BindTexture(uint32_t slot, TextureHandle handle)
    {
        if (m_state.SetTexture(slot, handle))
            ++m_stats.textureBinds;
    }

    void OpenGLRenderDevice::BindShader(ShaderHandle handle)
    {
        if (m_state.SetShader(handle))
            ++m_stats.shaderBinds;
    }

    void OpenGLRenderDevice::BindPipeline(PipelineHandle handle)
    {
        m_state.SetPipeline(handle);
    }

    void OpenGLRenderDevice::Draw(const DrawDesc& desc)
    {
        if (!m_initialized || desc.vertexCount == 0)
            return;
        ++m_stats.drawCalls;
        m_stats.triangles += desc.vertexCount / 3;
    }

    void OpenGLRenderDevice::DrawIndexed(const DrawIndexedDesc& desc)
    {
        if (!m_initialized || desc.indexCount == 0)
            return;
        ++m_stats.drawCalls;
        ++m_stats.indexedDrawCalls;
        m_stats.triangles += desc.indexCount / 3;
    }

    void OpenGLRenderDevice::BeginFrame()
    {
        if (m_initialized)
            m_stats.Reset();
    }

    void OpenGLRenderDevice::EndFrame() {}

    const RenderStats& OpenGLRenderDevice::GetStats() const { return m_stats; }
}
