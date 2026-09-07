#pragma once

#include "BMDGpuSkinningUBO.h"
#include "../Uniforms/BoneUBO.h"
#include "../../../dependencies/include/glad/glad.h"

namespace Render
{
    // Isolated GL transport for the BMD GPU-skinning palette.
    // This helper is intentionally not wired into the global renderer yet.
    // It can therefore be validated independently before replacing the
    // current glUniform4fv palette path.
    class BMDGpuSkinningUBOBinding
    {
    public:
        BMDGpuSkinningUBOBinding()
            : m_buffer(0)
        {
        }

        ~BMDGpuSkinningUBOBinding()
        {
            Release();
        }

        BMDGpuSkinningUBOBinding(const BMDGpuSkinningUBOBinding&) = delete;
        BMDGpuSkinningUBOBinding& operator=(const BMDGpuSkinningUBOBinding&) = delete;

        bool EnsureCreated()
        {
            if (m_buffer != 0)
                return true;

            glGenBuffers(1, &m_buffer);
            return m_buffer != 0;
        }

        bool Upload(const BonePalette& palette)
        {
            if (!EnsureCreated())
                return false;

            BMDGpuBoneBlock block;
            BuildBMDGpuBoneBlock(palette, block);

            glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
            glBufferData(GL_UNIFORM_BUFFER,
                         static_cast<GLsizeiptr>(sizeof(block)),
                         &block,
                         GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, BoneUBOSlot, m_buffer);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            return true;
        }

        void Bind() const
        {
            if (m_buffer != 0)
                glBindBufferBase(GL_UNIFORM_BUFFER, BoneUBOSlot, m_buffer);
        }

        void Release()
        {
            if (m_buffer != 0)
            {
                glDeleteBuffers(1, &m_buffer);
                m_buffer = 0;
            }
        }

        GLuint GetBuffer() const
        {
            return m_buffer;
        }

    private:
        GLuint m_buffer;
    };
}
