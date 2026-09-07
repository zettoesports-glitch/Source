#pragma once

#include "Renderer/Core/RenderTypes.h"

#include <string>
#include <unordered_map>

namespace MU::Render
{
    class ShaderManager
    {
    public:
        bool Initialize();
        void Shutdown();

        bool RegisterProgram(const std::string& name, ShaderHandle program);
        ShaderHandle FindProgram(const std::string& name) const;
        bool RemoveProgram(const std::string& name);
        void Clear();
        bool IsInitialized() const { return m_Initialized; }

    private:
        std::unordered_map<std::string, ShaderHandle> m_Programs;
        bool m_Initialized = false;
    };
}
