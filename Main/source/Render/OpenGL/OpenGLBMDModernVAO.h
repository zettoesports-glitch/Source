#pragma once

#include <cstdint>

struct BMDModernMeshData;

// Dedicated OpenGL VAO for the modern model shader contract. It intentionally
// does not reuse the legacy BMD VAO because the attribute locations differ.
class OpenGLBMDModernVAO
{
public:
    OpenGLBMDModernVAO();
    ~OpenGLBMDModernVAO();

    OpenGLBMDModernVAO(const OpenGLBMDModernVAO&) = delete;
    OpenGLBMDModernVAO& operator=(const OpenGLBMDModernVAO&) = delete;

    bool Upload(const BMDModernMeshData& mesh);
    void Bind() const;
    void Destroy();

    unsigned int GetVertexArray() const { return m_VertexArray; }
    std::uint32_t GetVertexCount() const { return m_VertexCount; }
    std::uint32_t GetIndexCount() const { return m_IndexCount; }
    bool IsValid() const { return m_VertexArray != 0 && m_IndexCount != 0; }

private:
    unsigned int m_VertexArray;
    unsigned int m_VertexBuffer;
    unsigned int m_IndexBuffer;
    std::uint32_t m_VertexCount;
    std::uint32_t m_IndexCount;
};
