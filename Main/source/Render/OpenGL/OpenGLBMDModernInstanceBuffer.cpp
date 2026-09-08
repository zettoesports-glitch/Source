#include "stdafx.h"
#include "OpenGLBMDModernInstanceBuffer.h"
#include "../Model/BMDModernInstance.h"

#include <cstddef>
#include <cstring>
#include <type_traits>

static_assert(std::is_standard_layout<BMDModernInstance>::value,
              "BMDModernInstance must remain standard-layout for offsetof");

namespace
{
    static void EnableFloatAttribute(unsigned int location,
                                     int components,
                                     std::size_t offset)
    {
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location,
                              components,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(BMDModernInstance),
                              reinterpret_cast<void*>(offset));
        glVertexAttribDivisor(location, 1);
    }
}

OpenGLBMDModernInstanceBuffer::OpenGLBMDModernInstanceBuffer()
    : m_Buffer(0)
    , m_InstanceCount(0)
{
}

OpenGLBMDModernInstanceBuffer::~OpenGLBMDModernInstanceBuffer()
{
    Destroy();
}

bool OpenGLBMDModernInstanceBuffer::UploadAndAttach(
    unsigned int vertexArray,
    const BMDModernInstance* instances,
    std::uint32_t instanceCount)
{
    if (vertexArray == 0 || instances == NULL || instanceCount == 0)
        return false;

    if (m_Buffer == 0)
    {
        glGenBuffers(1, &m_Buffer);
        if (m_Buffer == 0)
            return false;
    }

    const std::size_t byteCount =
        static_cast<std::size_t>(instanceCount) * sizeof(BMDModernInstance);
    const unsigned char* uploadBytes =
        reinterpret_cast<const unsigned char*>(instances);

    // ModernBMD is embedded inside the legacy renderer. Treat the generic
    // ARRAY_BUFFER and VAO bindings as borrowed state rather than resetting them
    // to zero after every upload, mirroring the explicit ownership rule used by
    // Sven's Core Profile BindState layer.
    GLint previousArrayBuffer = 0;
    GLint previousVertexArray = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousArrayBuffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);

    // Keep the existing orphan/upload behaviour when the instance changes,
    // but skip it entirely when consecutive meshes use the exact same instance
    // payload. This is safe because all VAOs reference this same buffer object.
    const bool sameUpload =
        m_LastUpload.size() == byteCount &&
        std::memcmp(m_LastUpload.data(), uploadBytes, byteCount) == 0;
    if (!sameUpload)
    {
        glBufferData(GL_ARRAY_BUFFER,
                     byteCount,
                     instances,
                     GL_DYNAMIC_DRAW);
        m_LastUpload.assign(uploadBytes, uploadBytes + byteCount);
    }

    // glVertexAttribPointer/glVertexAttribIPointer state belongs to the VAO and
    // captures the current GL_ARRAY_BUFFER object. Configure locations 6..12
    // only the first time each modern mesh VAO sees this shared instance buffer.
    // Re-uploading storage with glBufferData does not invalidate that VAO state.
    if (m_AttachedVertexArrays.insert(vertexArray).second)
    {
        glBindVertexArray(vertexArray);

        EnableFloatAttribute(6, 3, offsetof(BMDModernInstance, BodyOrigin));
        EnableFloatAttribute(7, 2, offsetof(BMDModernInstance, BodyScale));
        EnableFloatAttribute(8, 2, offsetof(BMDModernInstance, Data));
        EnableFloatAttribute(9, 4, offsetof(BMDModernInstance, Data2));
        EnableFloatAttribute(10, 4, offsetof(BMDModernInstance, BodyLight));
        EnableFloatAttribute(11, 4, offsetof(BMDModernInstance, ShadowLight));

        glEnableVertexAttribArray(12);
        glVertexAttribIPointer(12,
                               1,
                               GL_UNSIGNED_INT,
                               sizeof(BMDModernInstance),
                               reinterpret_cast<void*>(offsetof(BMDModernInstance, BoneIndex)));
        glVertexAttribDivisor(12, 1);
    }

    m_InstanceCount = instanceCount;

    glBindVertexArray(static_cast<GLuint>(previousVertexArray));
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previousArrayBuffer));
    return true;
}

void OpenGLBMDModernInstanceBuffer::Destroy()
{
    if (m_Buffer != 0)
    {
        glDeleteBuffers(1, &m_Buffer);
        m_Buffer = 0;
    }

    m_InstanceCount = 0;
    m_LastUpload.clear();
    m_AttachedVertexArrays.clear();
}