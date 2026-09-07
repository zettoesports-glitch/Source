#pragma once

#include <string>
#include <unordered_map>
#include "../Core/RenderTypes.h"

namespace Render
{
    class ShaderManager
    {
    public:
        ShaderManager() : m_nextHandle(1) {}

        ShaderHandle Register(const std::string& name)
        {
            std::unordered_map<std::string, ShaderHandle>::const_iterator it = m_byName.find(name);
            if (it != m_byName.end())
                return it->second;
            ShaderHandle handle = m_nextHandle++;
            m_byName[name] = handle;
            m_names[handle] = name;
            return handle;
        }

        ShaderHandle Find(const std::string& name) const
        {
            std::unordered_map<std::string, ShaderHandle>::const_iterator it = m_byName.find(name);
            return it != m_byName.end() ? it->second : InvalidShader;
        }

        const std::string& GetName(ShaderHandle handle) const
        {
            static const std::string empty;
            std::unordered_map<ShaderHandle, std::string>::const_iterator it = m_names.find(handle);
            return it != m_names.end() ? it->second : empty;
        }

        void Clear()
        {
            m_byName.clear();
            m_names.clear();
            m_nextHandle = 1;
        }

    private:
        ShaderHandle m_nextHandle;
        std::unordered_map<std::string, ShaderHandle> m_byName;
        std::unordered_map<ShaderHandle, std::string> m_names;
    };
}
