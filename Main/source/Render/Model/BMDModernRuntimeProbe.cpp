#include "stdafx.h"

#if jdk_shader_local330

#include <fstream>
#include <string>

namespace
{
    static std::string GetExecutablePath()
    {
        char path[MAX_PATH] = { 0 };
        const DWORD length = GetModuleFileNameA(NULL, path, MAX_PATH);
        if (length == 0 || length >= MAX_PATH)
            return std::string();
        return std::string(path, path + length);
    }

    static std::string GetDirectory(const std::string& path)
    {
        const std::string::size_type slash = path.find_last_of("\\/");
        if (slash == std::string::npos)
            return std::string();
        return path.substr(0, slash);
    }

    static std::string JoinPath(const std::string& root, const char* relative)
    {
        if (root.empty())
            return relative ? std::string(relative) : std::string();
        if (relative == NULL || relative[0] == '\0')
            return root;
        return root + "\\" + relative;
    }

    static bool FileExists(const std::string& path)
    {
        if (path.empty())
            return false;
        const DWORD attributes = GetFileAttributesA(path.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES &&
               (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    struct BMDModernRuntimeBootstrapProbe
    {
        BMDModernRuntimeBootstrapProbe()
        {
            const std::string exePath = GetExecutablePath();
            const std::string exeDirectory = GetDirectory(exePath);
            const std::string logPath = JoinPath(exeDirectory, "ModernBMD_boot.log");
            const std::string configPath = JoinPath(exeDirectory, "Data\\Custom\\config.ini");
            const std::string vertexShaderPath = JoinPath(exeDirectory, "Data\\Effect\\Modern\\BMDExperimental.vs");
            const std::string fragmentShaderPath = JoinPath(exeDirectory, "Data\\Effect\\Modern\\BMDExperimental.fs");

            char cwd[MAX_PATH] = { 0 };
            GetCurrentDirectoryA(MAX_PATH, cwd);

            const int experimental = GetPrivateProfileIntA(
                "ModernRenderer",
                "ExperimentalBMD",
                -1,
                configPath.c_str());

            std::ofstream log(logPath.c_str(), std::ios::out | std::ios::trunc);
            if (!log.is_open())
                return;

            log << "[ModernBMD-Probe] build=2026-09-07-complex-diagnostics-v1\n";
            log << "exe=" << exePath << "\n";
            log << "cwd=" << cwd << "\n";
            log << "config=" << configPath
                << " exists=" << (FileExists(configPath) ? 1 : 0)
                << " ExperimentalBMD=" << experimental << "\n";
            log << "vertexShader=" << vertexShaderPath
                << " exists=" << (FileExists(vertexShaderPath) ? 1 : 0) << "\n";
            log << "fragmentShader=" << fragmentShaderPath
                << " exists=" << (FileExists(fragmentShaderPath) ? 1 : 0) << "\n";
        }
    };

    static BMDModernRuntimeBootstrapProbe g_BMDModernRuntimeBootstrapProbe;
}

#endif // jdk_shader_local330
