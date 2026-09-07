#include "stdafx.h"
#include "Renderer/OpenGL/OpenGLRenderDevice.h"

#include <GL/glew.h>

namespace MU::Render
{
    bool OpenGLRenderDevice::Initialize(const RenderConfig& config)
    {
        if (m_Initialized)
            return true;

        const GLubyte* version = glGetString(GL_VERSION);
        if (version == nullptr)
            return false;

        m_Viewport.Width = config.Width;
        m_Viewport.Height = config.Height;
        m_BindState.Reset();
        m_Stats.Reset();
        m_Initialized = true;

        if (config.Width > 0 && config.Height > 0)
            glViewport(0, 0, config.Width, config.Height);

        return true;
    }

    void OpenGLRenderDevice::BeginFrame()
    {
        if (!m_Initialized)
            return;

        m_Stats.Reset();
    }

    void OpenGLRenderDevice::EndFrame()
    {
        if (!m_Initialized)
            return;
    }

    void OpenGLRenderDevice::Resize(int width, int height)
    {
        if (!m_Initialized)
            return;

        m_Viewport.Width = width;
        m_Viewport.Height = height;

        if (width > 0 && height > 0)
            glViewport(0, 0, width, height);
    }

    void OpenGLRenderDevice::Shutdown()
    {
        if (!m_Initialized)
            return;

        m_BindState.Reset();
        m_Stats.Reset();
        m_Viewport = RenderViewport{};
        m_Initialized = false;
    }
}
