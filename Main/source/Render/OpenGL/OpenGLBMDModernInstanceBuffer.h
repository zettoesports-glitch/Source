#pragma once

#include <cstdint>
#include <unordered_set>
#include <vector>

struct BMDModernInstance;

class OpenGLBMDModernInstanceBuffer
{
public:
    OpenGLBMDModernInstanceBuffer();
    ~OpenGLBMDModernInstanceBuffer();

    OpenGLBMDModernInstanceBuffer(const OpenGLBMDModernInstanceBuffer&) = delete;
    OpenGLBMDModernInstanceBuffer& operator=(const OpenGLBMDModernInstanceBuffer&) = delete;

    // Uploads instance data and installs locations 6..12 into the supplied
    // modern VAO. Attribute layout is cached per VAO because the GL VAO keeps
    // the buffer object association until that VAO is destroyed.
    bool UploadAndAttach(unsigned int vertexArray,
                         const BMDModernInstance* instances,
                         std::uint32_t instanceCount);

    void Destroy();

    unsigned int GetBuffer() const { return m_Buffer; }
    std::uint32_t GetInstanceCount() const { return m_InstanceCount; }

private:
    unsigned int m_Buffer;
    std::uint32_t m_InstanceCount;
    std::vector<unsigned char> m_LastUpload;
    std::unordered_set<unsigned int> m_AttachedVertexArrays;
};
