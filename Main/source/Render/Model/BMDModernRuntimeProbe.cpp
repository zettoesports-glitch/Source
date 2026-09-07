#include "stdafx.h"
#include "BMDModernRuntimeProbe.h"

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

    static std::string GetExecutableDirectory()
    {
        return GetDirectory(GetExecutablePath());
    }

    struct BMDModernRuntimeBootstrapProbe
    {
        BMDModernRuntimeBootstrapProbe()
        {
            const std::string exePath = GetExecutablePath();
            const std::string exeDirectory = GetDirectory(exePath);
            const std::string logPath = JoinPath(exeDirectory, "ModernBMD_boot.log");
            const std::string configPath = JoinPath(exeDirectory, "Data\\Custom\\config.ini");
            const std::string generatedVertexPath = JoinPath(exeDirectory, "Data\\Effect\\Modern\\Generated\\models\\texture.vs");
            const std::string generatedMatrixVertexPath = JoinPath(exeDirectory, "Data\\Effect\\Modern\\Generated\\models\\texture_matrix.vs");
            const std::string generatedPixelPath = JoinPath(exeDirectory, "Data\\Effect\\Modern\\Generated\\models\\texture.ps");
            const std::string experimentalVertexPath = JoinPath(exeDirectory, "Data\\Effect\\Modern\\BMDExperimental.vs");
            const std::string experimentalFragmentPath = JoinPath(exeDirectory, "Data\\Effect\\Modern\\BMDExperimental.fs");

            char cwd[MAX_PATH] = { 0 };
            GetCurrentDirectoryA(MAX_PATH, cwd);

            const int experimental = GetPrivateProfileIntA(
                "ModernRenderer", "ExperimentalBMD", -1, configPath.c_str());
            const int generated = GetPrivateProfileIntA(
                "ModernRenderer", "UseGeneratedShaders", 1, configPath.c_str());
            const int atlas = GetPrivateProfileIntA(
                "ModernRenderer", "AtlasMode", 1, configPath.c_str());
            const int matrixSkeleton = GetPrivateProfileIntA(
                "ModernRenderer", "MatrixSkeleton", 1, configPath.c_str());

            std::ofstream log(logPath.c_str(), std::ios::out | std::ios::trunc);
            if (!log.is_open())
                return;

            log << "[ModernBMD-Probe] build=2026-09-07-atlas-matrix-skeleton-v2\n";
            log << "exe=" << exePath << "\n";
            log << "cwd=" << cwd << "\n";
            log << "config=" << configPath
                << " exists=" << (FileExists(configPath) ? 1 : 0)
                << " ExperimentalBMD=" << experimental
                << " AtlasMode=" << atlas
                << " UseGeneratedShaders=" << generated
                << " MatrixSkeleton=" << matrixSkeleton << "\n";
            log << "generatedQPSVertex=" << generatedVertexPath
                << " exists=" << (FileExists(generatedVertexPath) ? 1 : 0) << "\n";
            log << "generatedMatrixVertex=" << generatedMatrixVertexPath
                << " exists=" << (FileExists(generatedMatrixVertexPath) ? 1 : 0) << "\n";
            log << "generatedPixel=" << generatedPixelPath
                << " exists=" << (FileExists(generatedPixelPath) ? 1 : 0) << "\n";
            log << "experimentalVertex=" << experimentalVertexPath
                << " exists=" << (FileExists(experimentalVertexPath) ? 1 : 0) << "\n";
            log << "experimentalFragment=" << experimentalFragmentPath
                << " exists=" << (FileExists(experimentalFragmentPath) ? 1 : 0) << "\n";
            log << "renderEntry=0\n";
        }
    };

    static BMDModernRuntimeBootstrapProbe g_BMDModernRuntimeBootstrapProbe;
}

void BMDModernProbeMarkRenderEntry()
{
    static bool marked = false;
    if (marked)
        return;
    marked = true;

    const std::string logPath = JoinPath(GetExecutableDirectory(), "ModernBMD_render.log");
    std::ofstream log(logPath.c_str(), std::ios::out | std::ios::trunc);
    if (!log.is_open())
        return;

    log << "[ModernBMD-Probe] CGMShaderBMD::Render reached\n";
}

#endif // jdk_shader_local330
