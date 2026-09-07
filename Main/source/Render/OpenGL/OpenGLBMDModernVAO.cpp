#include "stdafx.h"
#include "OpenGLBMDModernVAO.h"
#include "../Model/BMDModernMesh.h"

#include <cstddef>
#include <type_traits>

static_assert(std::is_standard_layout<BMDModernVertex>::value,
              "BMDModernVertex must remain standard-layout for offsetof");

OpenGLBMDModernVAO::OpenGLBMDModernVAO()
    : m_VertexArray(0)
    , m_VertexBuffer(0)
    , m_IndexBuffer(0)
    , m_VertexCount(0)
    , m_IndexCount(0)
{
}

OpenGLBMDModernVAO::~OpenGLBMDModernVAO()
{
    Destroy();
}

bool OpenGLBMDModernVAO::Upload(const BMDModernMeshData& mesh)
{
    if (mesh.Empty())
        return false;

    Destroy();

    glGenVertexArrays(1, &m_VertexArray);
    glBindVertexArray(m_VertexArray);

    glGenBuffers(1, &m_VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.Vertices.size() * sizeof(BMDModernVertex),
                 mesh.Vertices.data(),
                 GL_STATIC_DRAW);

    glGenBuffers(1, &m_IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.Indices.size() * sizeof(std::uint32_t),
                 mesh.Indices.data(),
                 GL_STATIC_DRAW);

    // Modern shader contract from VertexDefinitions.inc / generated GLSL:
    // 0 Position, 1 UV, 2 Normal, 3 uint2 Bones.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(BMDModernVertex),
                          reinterpret_cast<void*>(offsetof(BMDModernVertex, Position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(BMDModernVertex),
                          reinterpret_cast<void*>(offsetof(BMDModernVertex, UV)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
                          sizeof(BMDModernVertex),
                          reinterpret_cast<void*>(offsetof(BMDModernVertex, Normal)));

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 2, GL_UNSIGNED_INT,
                           sizeof(BMDModernVertex),
                           reinterpret_cast<void*>(offsetof(BMDModernVertex, Bones)));

    m_VertexCount = static_cast<std::uint32_t>(mesh.Vertices.size());
    m_IndexCount = static_cast<std::uint32_t>(mesh.Indices.size());

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

void OpenGLBMDModernVAO::Bind() const
{
    glBindVertexArray(m_VertexArray);
}

void OpenGLBMDModernVAO::Destroy()
{
    if (m_IndexBuffer != 0)
    {
        glDeleteBuffers(1, &m_IndexBuffer);
        m_IndexBuffer = 0;
    }

    if (m_VertexBuffer != 0)
    {
        glDeleteBuffers(1, &m_VertexBuffer);
        m_VertexBuffer = 0;
    }

    if (m_VertexArray != 0)
    {
        glDeleteVertexArrays(1, &m_VertexArray);
        m_VertexArray = 0;
    }

    m_VertexCount = 0;
    m_IndexCount = 0;
}
