#pragma once

#include <cstdint>

struct BMDModernInstance;

class OpenGLBMDModernInstanceBuffer
{
public:
    OpenGLBMDModernInstanceBuffer();
    ~OpenGLBMDModernInstanceBuffer();

    OpenGLBMDModernInstanceBuffer(const OpenGLBMDModernInstanceBuffer&) = delete;
    OpenGLBMDModernInstanceBuffer& operator=(const OpenGLBMDModernInstanceBuffer&) = delete;

    // Uploads instance data and installs locations 6..12 into the supplied
    // modern VAO. The buffer must outlive draws using that VAO.
    bool UploadAndAttach(unsigned int vertexArray,
                         const BMDModernInstance* instances,
                         std::uint32_t instanceCount);

    void Destroy();

    unsigned int GetBuffer() const { return m_Buffer; }
    std::uint32_t GetInstanceCount() const { return m_InstanceCount; }

private:
    unsigned int m_Buffer;
    std::uint32_t m_InstanceCount;
};
