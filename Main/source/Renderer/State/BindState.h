#pragma once

#include "Renderer/Core/RenderTypes.h"

#include <array>
#include <cstdint>

namespace MU::Render
{
    constexpr std::uint32_t MaxTextureUnits = 16;

    class BindState
    {
    public:
        void Reset()
        {
            m_Shader = InvalidShader;
            m_VertexArray = 0;
            m_ArrayBuffer = InvalidBuffer;
            m_IndexBuffer = InvalidBuffer;
            m_Framebuffer = 0;
            m_Textures.fill(InvalidTexture);
        }

        bool NeedsShader(ShaderHandle value) const { return m_Shader != value; }
        bool NeedsArrayBuffer(BufferHandle value) const { return m_ArrayBuffer != value; }
        bool NeedsIndexBuffer(BufferHandle value) const { return m_IndexBuffer != value; }
        bool NeedsTexture(std::uint32_t unit, TextureHandle value) const
        {
            return unit >= MaxTextureUnits || m_Textures[unit] != value;
        }

        void SetShader(ShaderHandle value) { m_Shader = value; }
        void SetVertexArray(std::uint32_t value) { m_VertexArray = value; }
        void SetArrayBuffer(BufferHandle value) { m_ArrayBuffer = value; }
        void SetIndexBuffer(BufferHandle value) { m_IndexBuffer = value; }
        void SetFramebuffer(std::uint32_t value) { m_Framebuffer = value; }
        void SetTexture(std::uint32_t unit, TextureHandle value)
        {
            if (unit < MaxTextureUnits)
                m_Textures[unit] = value;
        }

        ShaderHandle GetShader() const { return m_Shader; }
        std::uint32_t GetVertexArray() const { return m_VertexArray; }
        BufferHandle GetArrayBuffer() const { return m_ArrayBuffer; }
        BufferHandle GetIndexBuffer() const { return m_IndexBuffer; }
        std::uint32_t GetFramebuffer() const { return m_Framebuffer; }
        TextureHandle GetTexture(std::uint32_t unit) const
        {
            return unit < MaxTextureUnits ? m_Textures[unit] : InvalidTexture;
        }

    private:
        ShaderHandle m_Shader = InvalidShader;
        std::uint32_t m_VertexArray = 0;
        BufferHandle m_ArrayBuffer = InvalidBuffer;
        BufferHandle m_IndexBuffer = InvalidBuffer;
        std::uint32_t m_Framebuffer = 0;
        std::array<TextureHandle, MaxTextureUnits> m_Textures{};
    };
}
