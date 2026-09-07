#include "stdafx.h"
#include "OpenGLBMDModernInstanceBuffer.h"
#include "../Model/BMDModernInstance.h"

#include <cstddef>
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
        glGenBuffers(1, &m_Buffer);

    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<size_t>(instanceCount) * sizeof(BMDModernInstance),
                 instances,
                 GL_DYNAMIC_DRAW);

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

    m_InstanceCount = instanceCount;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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
}
