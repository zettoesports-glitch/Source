#pragma once

#include <cstdint>
#include "../Core/RenderTypes.h"

namespace Render
{
    class BindState
    {
    public:
        BindState() { Reset(); }

        void Reset()
        {
            m_program = InvalidShader;
            m_vao = 0;
            m_vertexBuffer = InvalidBuffer;
            m_indexBuffer = InvalidBuffer;
            m_pipeline = InvalidPipeline;
            m_indexType = IndexType::UInt32;
            for (uint32_t i = 0; i < MaxTextureUnits; ++i)
                m_textures[i] = InvalidTexture;
        }

        static const uint32_t MaxTextureUnits = 8;

        bool SetShader(ShaderHandle value) { return Set(m_program, value); }
        bool SetVertexBuffer(BufferHandle value) { return Set(m_vertexBuffer, value); }
        bool SetIndexBuffer(BufferHandle value) { return Set(m_indexBuffer, value); }
        bool SetPipeline(PipelineHandle value) { return Set(m_pipeline, value); }
        bool SetVAO(uint32_t value) { return Set(m_vao, value); }

        bool SetTexture(uint32_t slot, TextureHandle value)
        {
            if (slot >= MaxTextureUnits || m_textures[slot] == value)
                return false;
            m_textures[slot] = value;
            return true;
        }

        bool SetIndexType(IndexType value)
        {
            if (m_indexType == value)
                return false;
            m_indexType = value;
            return true;
        }

        ShaderHandle GetShader() const { return m_program; }
        BufferHandle GetVertexBuffer() const { return m_vertexBuffer; }
        BufferHandle GetIndexBuffer() const { return m_indexBuffer; }
        PipelineHandle GetPipeline() const { return m_pipeline; }
        uint32_t GetVAO() const { return m_vao; }
        TextureHandle GetTexture(uint32_t slot) const { return slot < MaxTextureUnits ? m_textures[slot] : InvalidTexture; }
        IndexType GetIndexType() const { return m_indexType; }

    private:
        template<typename T>
        static bool Set(T& current, const T& value)
        {
            if (current == value)
                return false;
            current = value;
            return true;
        }

        ShaderHandle m_program;
        uint32_t m_vao;
        BufferHandle m_vertexBuffer;
        BufferHandle m_indexBuffer;
        PipelineHandle m_pipeline;
        TextureHandle m_textures[MaxTextureUnits];
        IndexType m_indexType;
    };
}
