#include "stdafx.h"
#include "Renderer/Shader/ShaderManager.h"

namespace MU::Render
{
    bool ShaderManager::Initialize()
    {
        m_Programs.clear();
        m_Initialized = true;
        return true;
    }

    void ShaderManager::Shutdown()
    {
        m_Programs.clear();
        m_Initialized = false;
    }

    bool ShaderManager::RegisterProgram(const std::string& name, ShaderHandle program)
    {
        if (!m_Initialized || name.empty() || program == InvalidShader)
            return false;

        m_Programs[name] = program;
        return true;
    }

    ShaderHandle ShaderManager::FindProgram(const std::string& name) const
    {
        const auto it = m_Programs.find(name);
        return it != m_Programs.end() ? it->second : InvalidShader;
    }

    bool ShaderManager::RemoveProgram(const std::string& name)
    {
        return m_Programs.erase(name) != 0;
    }

    void ShaderManager::Clear()
    {
        m_Programs.clear();
    }
}
