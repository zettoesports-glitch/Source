#include "stdafx.h"
#include "BMDModernRuntime.h"

#if jdk_shader_local330

#include "BMDModernInstanceBuilder.h"
#include "BMDModernMesh.h"
#include "BMDModernSkeletonAtlas.h"
#include "../OpenGL/OpenGLBMDModernBindings.h"
#include "../OpenGL/OpenGLBMDModernInstanceBuffer.h"
#include "../OpenGL/OpenGLBMDModernVAO.h"
#include "../OpenGL/OpenGLShaderGlobalConstants.h"
#include "../OpenGL/OpenGLSkeletonTexture.h"
#include "ZzzBMD.h"
#include "ZzzTexture.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    static void ModernLog(const char* message)
    {
        if (message == NULL)
            return;

        OutputDebugStringA("[ModernBMD] ");
        OutputDebugStringA(message);
        OutputDebugStringA("\n");

        std::ofstream logFile("Data\\ModernBMD.log", std::ios::out | std::ios::app);
        if (logFile.is_open())
            logFile << "[ModernBMD] " << message << '\n';
    }

    static std::string ReadTextFile(const char* path)
    {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open())
            return std::string();

        std::ostringstream stream;
        stream << file.rdbuf();
        return stream.str();
    }

    static std::string PrepareShaderSource(const std::string& source)
    {
        if (source.empty())
            return source;

        if (source.find("#version") != std::string::npos)
            return source;

        return std::string("#version 460 core\n") + source;
    }

    static GLuint CompileStage(GLenum type, const std::string& source, const char* label)
    {
        if (source.empty())
            return 0;

        const std::string prepared = PrepareShaderSource(source);
        const char* text = prepared.c_str();
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &text, NULL);
        glCompileShader(shader);

        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (compiled != GL_TRUE)
        {
            char log[2048] = { 0 };
            glGetShaderInfoLog(shader, static_cast<GLsizei>(sizeof(log) - 1), NULL, log);
            const std::string message = std::string(label ? label : "shader") + ": " + log;
            ModernLog(message.c_str());
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    static GLuint LoadProgram(const char* vertexPath, const char* fragmentPath)
    {
        const std::string vertexSource = ReadTextFile(vertexPath);
        const std::string fragmentSource = ReadTextFile(fragmentPath);
        if (vertexSource.empty() || fragmentSource.empty())
            return 0;

        GLuint vertex = CompileStage(GL_VERTEX_SHADER, vertexSource, vertexPath);
        if (vertex == 0)
            return 0;

        GLuint fragment = CompileStage(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
        if (fragment == 0)
        {
            glDeleteShader(vertex);
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        GLint linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE)
        {
            char log[2048] = { 0 };
            glGetProgramInfoLog(program, static_cast<GLsizei>(sizeof(log) - 1), NULL, log);
            const std::string message = std::string("program link: ") + log;
            ModernLog(message.c_str());
            glDeleteProgram(program);
            return 0;
        }

        return program;
    }

    static void DeleteProgram(GLuint& program)
    {
        if (program != 0)
        {
            glDeleteProgram(program);
            program = 0;
        }
    }

    static void SetIdentity(float* matrix)
    {
        if (matrix == NULL)
            return;

        std::memset(matrix, 0, sizeof(float) * 16u);
        matrix[0] = 1.0f;
        matrix[5] = 1.0f;
        matrix[10] = 1.0f;
        matrix[15] = 1.0f;
    }

    static void MultiplyColumnMajor4x4(const float* a, const float* b, float* out)
    {
        for (int column = 0; column < 4; ++column)
        {
            for (int row = 0; row < 4; ++row)
            {
                float value = 0.0f;
                for (int k = 0; k < 4; ++k)
                    value += a[k * 4 + row] * b[column * 4 + k];
                out[column * 4 + row] = value;
            }
        }
    }

    static int MaterialFlagsWithoutDepth(const OGL330MODEL::RenderMeshVAO& command)
    {
        return command.m_FlagRender & ~RENDER_NODEPTH;
    }

    static bool IsChrome01Material(const OGL330MODEL::RenderMeshVAO& command)
    {
        return MaterialFlagsWithoutDepth(command) == (RENDER_CHROME | RENDER_BRIGHT);
    }

    static bool IsChrome04Material(const OGL330MODEL::RenderMeshVAO& command)
    {
        return MaterialFlagsWithoutDepth(command) == (RENDER_CHROME4 | RENDER_BRIGHT);
    }

    static bool IsChrome05Material(const OGL330MODEL::RenderMeshVAO& command)
    {
        return MaterialFlagsWithoutDepth(command) ==
            (RENDER_CHROME5 | RENDER_TEXTURE | RENDER_BRIGHT);
    }

    static bool IsMetalMaterial(const OGL330MODEL::RenderMeshVAO& command)
    {
        return MaterialFlagsWithoutDepth(command) == (RENDER_METAL | RENDER_BRIGHT);
    }

    static void BuildGeneratedGlobals(const OGL330MODEL::RenderMeshVAO& command,
                                      const float* projection,
                                      const float* view,
                                      Render::ShaderGlobalConstants& constants)
    {
        std::memset(&constants, 0, sizeof(constants));
        SetIdentity(constants.Shadow);
        SetIdentity(constants.Game2D);
        constants.BillboardQ[3] = 1.0f;

        MultiplyColumnMajor4x4(projection, view, constants.Game3D);

        constants.LightPosition[0] = command.m_lightPosition.x;
        constants.LightPosition[1] = command.m_lightPosition.y;
        constants.LightPosition[2] = command.m_lightPosition.z;
        constants.WorldTime = static_cast<float>(WorldTime);

        // Use command-time material phases rather than recomputing them during
        // delayed flushes. This keeps material animation tied to the same immutable
        // render snapshot as the bone palette/body transform.
        if (IsChrome01Material(command))
        {
            constants.Wave[0] = command.m_setting1.z;
            constants.Wave[1] = command.m_setting1.z * 2.0f;
        }
        else if (IsChrome04Material(command) || IsChrome05Material(command))
        {
            constants.ChromeWave[0] = command.m_setting1.x;
            constants.ChromeWave[1] = command.m_setting1.y;
            constants.ChromeWave[2] = command.m_setting1.z;
            constants.Wave[0] = command.m_setting1.w;
        }
    }

    static std::uint64_t HashSkeletonPose(const float* matrices, std::uint32_t boneCount)
    {
        if (matrices == NULL || boneCount == 0)
            return 0;

        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(matrices);
        const size_t byteCount = static_cast<size_t>(boneCount) * 12u * sizeof(float);
        std::uint64_t hash = 1469598103934665603ull;
        for (size_t i = 0; i < byteCount; ++i)
        {
            hash ^= static_cast<std::uint64_t>(bytes[i]);
            hash *= 1099511628211ull;
        }
        return hash;
    }

    static std::uint32_t GetPaletteBoneCount(const OGL330MODEL::RenderMeshVAO& command)
    {
        if (!command.m_BonePalette || command.m_BonePalette->empty() ||
            (command.m_BonePalette->size() % 12u) != 0u)
        {
            return 0;
        }

        const size_t count = command.m_BonePalette->size() / 12u;
        if (count > static_cast<size_t>(MAX_BONES))
            return MAX_BONES;
        return static_cast<std::uint32_t>(count);
    }

    // The legacy u_Bones palette bakes BodyScale/BodyOrigin into every affine
    // matrix. Generated model shaders expect those values as per-instance
    // attributes instead. Recover a skeleton-only palette from the immutable
    // transform snapshot stored in this render command. Never read mutable BMD
    // transform fields here: the BMD asset may already be serving another object.
    static bool BuildModernSkeletonPalette(const OGL330MODEL::RenderMeshVAO& command,
                                           std::vector<float>& out)
    {
        out.clear();
        if (!command.m_BonePalette || command.m_BonePalette->empty())
            return false;

        out.assign(command.m_BonePalette->begin(), command.m_BonePalette->end());

        if (command.m_ModernTranslate)
        {
            const float bodyScale = command.m_ModernBodyScale;
            if (!std::isfinite(bodyScale) || std::fabs(bodyScale) <= 0.000001f)
                return false;

            const float invScale = 1.0f / bodyScale;
            const float origin[3] =
            {
                command.m_ModernBodyOrigin.x,
                command.m_ModernBodyOrigin.y,
                command.m_ModernBodyOrigin.z
            };

            const size_t boneCount = out.size() / 12u;
            for (size_t bone = 0; bone < boneCount; ++bone)
            {
                float* matrix = out.data() + bone * 12u;
                for (int row = 0; row < 3; ++row)
                {
                    float* affineRow = matrix + row * 4;
                    affineRow[0] *= invScale;
                    affineRow[1] *= invScale;
                    affineRow[2] *= invScale;
                    affineRow[3] = (affineRow[3] - origin[row]) * invScale;
                }
            }
        }

        // BMD::Animation intentionally skips Dummy bones, so shared/reused bone
        // buffers can retain an older model's transform in those slots. Never
        // upload that stale data to the modern atlas. Sanitize only slots that
        // are both inside this model's NumBones and inside the captured palette;
        // do not touch unused tail storage because callers can provide shorter
        // palettes. This mirrors the stale-bone protection used by the Core
        // renderer while preserving the legacy path byte-for-byte.
        BMD* model = command.m_OldBMD;
        if (model != NULL && model->Bones != NULL && model->NumBones > 0)
        {
            const size_t paletteBoneCount = out.size() / 12u;
            size_t modelBoneCount = static_cast<size_t>(model->NumBones);
            if (modelBoneCount > paletteBoneCount)
                modelBoneCount = paletteBoneCount;

            bool sanitizedAny = false;
            for (size_t bone = 0; bone < modelBoneCount; ++bone)
            {
                if (!model->Bones[bone].Dummy)
                    continue;

                float* matrix = out.data() + bone * 12u;
                std::memset(matrix, 0, sizeof(float) * 12u);
                matrix[0] = 1.0f;
                matrix[5] = 1.0f;
                matrix[10] = 1.0f;
                sanitizedAny = true;
            }

            if (sanitizedAny)
            {
                static bool logged = false;
                if (!logged)
                {
                    logged = true;
                    ModernLog("modern skeleton palette sanitizes Dummy bones to identity within captured NumBones; legacy palette remains unchanged");
                }
            }
        }

        return true;
    }

    struct MeshKey
    {
        BMD* Model;
        int MeshIndex;

        bool operator==(const MeshKey& other) const
        {
            return Model == other.Model && MeshIndex == other.MeshIndex;
        }
    };

    struct MeshKeyHash
    {
        size_t operator()(const MeshKey& key) const
        {
            return (reinterpret_cast<size_t>(key.Model) >> 4) ^
                   (static_cast<size_t>(key.MeshIndex) * static_cast<size_t>(2654435761u));
        }
    };

    struct ModernMeshGpu
    {
        OpenGLBMDModernVAO Vao;
    };
}

struct BMDModernRuntime::Impl
{
    enum DiagnosticBits
    {
        DiagnosticBasic = 1u << 0,
        DiagnosticFilter = 1u << 1,
        DiagnosticMaterial = 1u << 2,
        DiagnosticMesh = 1u << 3,
        DiagnosticSkeleton = 1u << 4,
        DiagnosticInstance = 1u << 5,
        DiagnosticBindings = 1u << 6,
        DiagnosticAtlas = 1u << 7,
        DiagnosticGlobals = 1u << 8
    };

    enum class ProgramMode
    {
        None,
        Generated,
        Experimental
    };

    enum class MaterialMode
    {
        Unsupported,
        Texture,
        TextureBright,
        ShadowTexture,
        Color,
        BrightColor,
        Chrome01,
        Chrome04,
        Chrome05,
        Metal
    };

    bool Enabled;
    bool AtlasMode;
    bool UseGeneratedShaders;
    bool MatrixSkeleton;
    bool Diagnostics;
    int DiagnosticLimit;
    int DiagnosticMessages;
    int DrawFailureMessages;
    static const int DrawFailureLimit = 8;
    int MinBones;
    int MinActions;
    int MaxAtlasPoses;
    char TargetName[64];

    bool ProgramAttempted;
    GLuint Program;
    GLuint ShadowTextureProgram;
    GLuint ColorProgram;
    GLuint ChromeProgram;
    GLuint Chrome04Program;
    GLuint Chrome05Program;
    GLuint MetalProgram;
    ProgramMode Mode;
    GLint ProjectionLocation;
    GLint ViewLocation;

    bool BatchPrepared;
    bool AtlasUploadLogged;
    bool MultiPoseLogged;
    bool TransformSnapshotLogged;
    bool MaterialProgramsLogged;
    BMD* SelectedModel;
    BMD* FirstModernModel;
    bool FirstPoseHashValid;
    bool FirstPoseChangeLogged;
    std::uint64_t FirstPoseHash;
    std::uint64_t SuccessfulDraws;

    BMDModernSkeletonAtlas Atlas;
    OpenGLSkeletonTexture SkeletonTexture;
    OpenGLBMDModernBindings Bindings;
    OpenGLBMDModernInstanceBuffer InstanceBuffer;
    OpenGLShaderGlobalConstants GlobalConstants;
    std::unordered_map<MeshKey, std::unique_ptr<ModernMeshGpu>, MeshKeyHash> Meshes;
    std::unordered_map<BMD*, unsigned int> DiagnosticMasks;

    Impl()
        : Enabled(GetPrivateProfileIntA("ModernRenderer", "ExperimentalBMD", 0,
                                        ".\\Data\\Custom\\config.ini") != 0)
        , AtlasMode(GetPrivateProfileIntA("ModernRenderer", "AtlasMode", 1,
                                          ".\\Data\\Custom\\config.ini") != 0)
        , UseGeneratedShaders(GetPrivateProfileIntA("ModernRenderer", "UseGeneratedShaders", 1,
                                                     ".\\Data\\Custom\\config.ini") != 0)
        , MatrixSkeleton(GetPrivateProfileIntA("ModernRenderer", "MatrixSkeleton", 1,
                                               ".\\Data\\Custom\\config.ini") != 0)
        , Diagnostics(GetPrivateProfileIntA("ModernRenderer", "Diagnostics", 1,
                                            ".\\Data\\Custom\\config.ini") != 0)
        , DiagnosticLimit(GetPrivateProfileIntA("ModernRenderer", "DiagnosticLimit", 24,
                                                ".\\Data\\Custom\\config.ini"))
        , DiagnosticMessages(0)
        , DrawFailureMessages(0)
        , MinBones(GetPrivateProfileIntA("ModernRenderer", "MinBones", 20,
                                         ".\\Data\\Custom\\config.ini"))
        , MinActions(GetPrivateProfileIntA("ModernRenderer", "MinActions", 2,
                                           ".\\Data\\Custom\\config.ini"))
        , MaxAtlasPoses(GetPrivateProfileIntA("ModernRenderer", "AtlasMaxPoses", 32,
                                              ".\\Data\\Custom\\config.ini"))
        , ProgramAttempted(false)
        , Program(0)
        , ShadowTextureProgram(0)
        , ColorProgram(0)
        , ChromeProgram(0)
        , Chrome04Program(0)
        , Chrome05Program(0)
        , MetalProgram(0)
        , Mode(ProgramMode::None)
        , ProjectionLocation(-1)
        , ViewLocation(-1)
        , BatchPrepared(false)
        , AtlasUploadLogged(false)
        , MultiPoseLogged(false)
        , TransformSnapshotLogged(false)
        , MaterialProgramsLogged(false)
        , SelectedModel(NULL)
        , FirstModernModel(NULL)
        , FirstPoseHashValid(false)
        , FirstPoseChangeLogged(false)
        , FirstPoseHash(0)
        , SuccessfulDraws(0)
        , Atlas(MatrixSkeleton ? SkeletonBuffer::StorageMode::Matrix4x4
                               : SkeletonBuffer::StorageMode::QuaternionPositionScale)
    {
        TargetName[0] = '\0';
        GetPrivateProfileStringA("ModernRenderer", "TargetName", "",
                                 TargetName, static_cast<DWORD>(sizeof(TargetName)),
                                 ".\\Data\\Custom\\config.ini");

        if (MinBones < 2)
            MinBones = 2;
        if (MinBones > MAX_BONES)
            MinBones = MAX_BONES;
        if (MinActions < 1)
            MinActions = 1;
        if (MaxAtlasPoses < 1)
            MaxAtlasPoses = 1;
        if (MaxAtlasPoses > 256)
            MaxAtlasPoses = 256;
        if (DiagnosticLimit < 1)
            DiagnosticLimit = 1;
        if (DiagnosticLimit > 128)
            DiagnosticLimit = 128;

        if (Enabled)
        {
            char message[512] = { 0 };
            sprintf_s(message,
                      "ExperimentalBMD=1; atlas runtime enabled (AtlasMode=%d, UseGeneratedShaders=%d, SkeletonEncoding=%s, MinBones=%d, MinActions=%d, AtlasMaxPoses=%d, TargetName=%s, Diagnostics=%d)",
                      AtlasMode ? 1 : 0,
                      UseGeneratedShaders ? 1 : 0,
                      MatrixSkeleton ? "Matrix4x4" : "QPS",
                      MinBones,
                      MinActions,
                      MaxAtlasPoses,
                      TargetName[0] != '\0' ? TargetName : "<any>",
                      Diagnostics ? 1 : 0);
            ModernLog(message);

            if (AtlasMode && MinActions > 1)
                ModernLog("AtlasMode uses captured bone palettes; MinActions is ignored so animated player equipment with one local action can participate");
            if (MatrixSkeleton)
                ModernLog("Matrix4x4 skeleton encoding selected for compatibility; QPS remains available with MatrixSkeleton=0");
        }
        else
        {
            ModernLog("ExperimentalBMD=0; legacy renderer only");
        }
    }

    ~Impl()
    {
        DeleteProgram(MetalProgram);
        DeleteProgram(Chrome05Program);
        DeleteProgram(Chrome04Program);
        DeleteProgram(ChromeProgram);
        DeleteProgram(ColorProgram);
        DeleteProgram(ShadowTextureProgram);
        DeleteProgram(Program);
    }

    const char* EncodingName() const
    {
        return MatrixSkeleton ? "matrix4x4" : "qps";
    }

    MaterialMode ClassifyMaterial(const OGL330MODEL::RenderMeshVAO& command) const
    {
        const int flags = MaterialFlagsWithoutDepth(command);
        if (flags == RENDER_TEXTURE)
            return MaterialMode::Texture;
        if (flags == (RENDER_TEXTURE | RENDER_BRIGHT))
            return MaterialMode::TextureBright;
        if (MatrixSkeleton && flags == (RENDER_SHADOWMAP | RENDER_TEXTURE))
            return MaterialMode::ShadowTexture;

        // Material overlays remain Matrix4x4-only during this rollout. QPS keeps
        // them on the legacy path so MatrixSkeleton=0 remains a safe rollback.
        if (MatrixSkeleton && flags == RENDER_COLOR)
            return MaterialMode::Color;
        if (MatrixSkeleton && flags == RENDER_BRIGHT)
            return MaterialMode::BrightColor;
        if (MatrixSkeleton && flags == (RENDER_CHROME | RENDER_BRIGHT))
            return MaterialMode::Chrome01;
        if (MatrixSkeleton && flags == (RENDER_CHROME4 | RENDER_BRIGHT))
            return MaterialMode::Chrome04;
        if (MatrixSkeleton && flags ==
            (RENDER_CHROME5 | RENDER_TEXTURE | RENDER_BRIGHT))
            return MaterialMode::Chrome05;
        if (MatrixSkeleton && flags == (RENDER_METAL | RENDER_BRIGHT))
            return MaterialMode::Metal;

        return MaterialMode::Unsupported;
    }

    bool IsMaterialEncodingSupported(const OGL330MODEL::RenderMeshVAO& command,
                                     MaterialMode material) const
    {
        if (material == MaterialMode::Unsupported)
            return false;

        if (material == MaterialMode::Texture ||
            material == MaterialMode::TextureBright ||
            material == MaterialMode::ShadowTexture)
        {
            const bool baseUV =
                command.m_meshUV.x == 0.0f &&
                command.m_meshUV.y == 0.0f &&
                command.m_meshUV.z == 0.0f;
            const bool blendStreamUV =
                command.m_meshUV.z == 1.0f &&
                std::isfinite(command.m_meshUV.x) &&
                std::isfinite(command.m_meshUV.y);
            return baseUV || blendStreamUV;
        }

        return true;
    }

    GLuint ProgramForMaterial(MaterialMode material) const
    {
        switch (material)
        {
        case MaterialMode::Texture:
        case MaterialMode::TextureBright:
            return Program;
        case MaterialMode::ShadowTexture: return ShadowTextureProgram;
        case MaterialMode::Color:
        case MaterialMode::BrightColor: return ColorProgram;
        case MaterialMode::Chrome01: return ChromeProgram;
        case MaterialMode::Chrome04: return Chrome04Program;
        case MaterialMode::Chrome05: return Chrome05Program;
        case MaterialMode::Metal: return MetalProgram;
        default: return 0;
        }
    }

    bool ConfigureCommonProgram(GLuint candidate)
    {
        if (candidate == 0)
            return false;

        if (!Bindings.ConfigureProgram(candidate))
            return false;

        return true;
    }

    bool ConfigureGeneratedProgram(GLuint candidate)
    {
        if (!ConfigureCommonProgram(candidate))
            return false;
        if (!GlobalConstants.ConfigureProgram(candidate))
            return false;
        return true;
    }

    bool TryGeneratedProgram()
    {
        const char* vertexPath = MatrixSkeleton
            ? "Data\\Effect\\Modern\\Generated\\models\\texture_matrix.vs"
            : "Data\\Effect\\Modern\\Generated\\models\\texture.vs";
        const char* fragmentPath = "Data\\Effect\\Modern\\Generated\\models\\texture.ps";

        GLuint textureCandidate = LoadProgram(vertexPath, fragmentPath);
        if (textureCandidate == 0)
            return false;

        GLuint shadowTextureCandidate = 0;
        GLuint colorCandidate = 0;
        GLuint chromeCandidate = 0;
        GLuint chrome04Candidate = 0;
        GLuint chrome05Candidate = 0;
        GLuint metalCandidate = 0;
        if (MatrixSkeleton)
        {
            shadowTextureCandidate = LoadProgram(
                "Data\\Effect\\Modern\\Generated\\models\\shadow_texture_matrix.vs",
                fragmentPath);
            colorCandidate = LoadProgram(
                "Data\\Effect\\Modern\\Generated\\models\\color_matrix.vs",
                "Data\\Effect\\Modern\\Generated\\models\\color.ps");
            chromeCandidate = LoadProgram(
                "Data\\Effect\\Modern\\Generated\\models\\chrome01_matrix.vs",
                fragmentPath);
            chrome04Candidate = LoadProgram(
                "Data\\Effect\\Modern\\Generated\\models\\chrome04_matrix.vs",
                fragmentPath);
            chrome05Candidate = LoadProgram(
                "Data\\Effect\\Modern\\Generated\\models\\chrome05_matrix.vs",
                fragmentPath);
            metalCandidate = LoadProgram(
                "Data\\Effect\\Modern\\Generated\\models\\metal_matrix.vs",
                fragmentPath);

            if (shadowTextureCandidate == 0 || colorCandidate == 0 ||
                chromeCandidate == 0 || chrome04Candidate == 0 ||
                chrome05Candidate == 0 || metalCandidate == 0)
            {
                DeleteProgram(metalCandidate);
                DeleteProgram(chrome05Candidate);
                DeleteProgram(chrome04Candidate);
                DeleteProgram(chromeCandidate);
                DeleteProgram(colorCandidate);
                DeleteProgram(shadowTextureCandidate);
                DeleteProgram(textureCandidate);
                ModernLog("generated Matrix4x4 ShadowTexture/Color/Chrome01/Chrome04/Chrome05/Metal material program unavailable; keeping complete modern batch on legacy renderer");
                return false;
            }
        }

        if (!GlobalConstants.Initialize() ||
            !ConfigureGeneratedProgram(textureCandidate) ||
            (MatrixSkeleton && !ConfigureGeneratedProgram(shadowTextureCandidate)) ||
            (MatrixSkeleton && !ConfigureGeneratedProgram(colorCandidate)) ||
            (MatrixSkeleton && !ConfigureGeneratedProgram(chromeCandidate)) ||
            (MatrixSkeleton && !ConfigureGeneratedProgram(chrome04Candidate)) ||
            (MatrixSkeleton && !ConfigureGeneratedProgram(chrome05Candidate)) ||
            (MatrixSkeleton && !ConfigureGeneratedProgram(metalCandidate)))
        {
            DeleteProgram(metalCandidate);
            DeleteProgram(chrome05Candidate);
            DeleteProgram(chrome04Candidate);
            DeleteProgram(chromeCandidate);
            DeleteProgram(colorCandidate);
            DeleteProgram(shadowTextureCandidate);
            DeleteProgram(textureCandidate);
            return false;
        }

        Program = textureCandidate;
        ShadowTextureProgram = shadowTextureCandidate;
        ColorProgram = colorCandidate;
        ChromeProgram = chromeCandidate;
        Chrome04Program = chrome04Candidate;
        Chrome05Program = chrome05Candidate;
        MetalProgram = metalCandidate;
        Mode = ProgramMode::Generated;
        ProjectionLocation = -1;
        ViewLocation = -1;

        // Leave the binding helper configured for the base program. It will be
        // switched only when a material-specific draw actually occurs.
        Bindings.ConfigureProgram(Program);

        char message[512] = { 0 };
        sprintf_s(message,
                  "generated OpenGL model programs ready (texture, texture-bright%s%s%s%s%s%s; source contract: vulkan-main HLSL + legacy VBO shadow projection -> GLSL, skeleton=%s)",
                  MatrixSkeleton ? ", shadow-texture" : "",
                  MatrixSkeleton ? ", color, color-bright" : "",
                  MatrixSkeleton ? ", chrome01" : "",
                  MatrixSkeleton ? ", chrome04" : "",
                  MatrixSkeleton ? ", chrome05" : "",
                  MatrixSkeleton ? ", metal" : "",
                  EncodingName());
        ModernLog(message);
        return true;
    }

    bool TryExperimentalProgram()
    {
        // BMDExperimental.vs is the original QPS first-light shader. Never pair
        // it with a Matrix4x4 atlas because its RequestBone() consumes 2 texels.
        if (MatrixSkeleton)
            return false;

        GLuint candidate = LoadProgram("Data\\Effect\\Modern\\BMDExperimental.vs",
                                       "Data\\Effect\\Modern\\BMDExperimental.fs");
        if (candidate == 0)
            return false;

        if (!ConfigureCommonProgram(candidate))
        {
            glDeleteProgram(candidate);
            return false;
        }

        const GLint projection = glGetUniformLocation(candidate, "uProj");
        const GLint view = glGetUniformLocation(candidate, "uView");
        if (projection < 0 || view < 0)
        {
            glDeleteProgram(candidate);
            return false;
        }

        Program = candidate;
        ShadowTextureProgram = 0;
        ColorProgram = 0;
        ChromeProgram = 0;
        Chrome04Program = 0;
        Chrome05Program = 0;
        MetalProgram = 0;
        Mode = ProgramMode::Experimental;
        ProjectionLocation = projection;
        ViewLocation = view;
        ModernLog("experimental shader program ready (generated shader fallback, skeleton=qps)");
        return true;
    }

    bool EnsureProgram()
    {
        if (Program != 0)
            return true;
        if (ProgramAttempted)
            return false;

        ProgramAttempted = true;

        if (UseGeneratedShaders)
        {
            if (TryGeneratedProgram())
                return true;

            if (MatrixSkeleton)
                ModernLog("generated Matrix4x4 model shader set unavailable/incompatible; legacy fallback only for modern batch");
            else
                ModernLog("generated QPS texture shader unavailable/incompatible; trying BMDExperimental fallback");
        }

        if (TryExperimentalProgram())
            return true;

        ModernLog("no compatible modern BMD shader program available; legacy fallback only");
        Mode = ProgramMode::None;
        Program = 0;
        ShadowTextureProgram = 0;
        ColorProgram = 0;
        ChromeProgram = 0;
        Chrome04Program = 0;
        Chrome05Program = 0;
        MetalProgram = 0;
        return false;
    }

    void LogDiagnostic(BMD* model,
                       const OGL330MODEL::RenderMeshVAO& command,
                       unsigned int bit,
                       const char* reason)
    {
        if (!Diagnostics || model == NULL || reason == NULL ||
            DiagnosticMessages >= DiagnosticLimit)
        {
            return;
        }

        unsigned int& mask = DiagnosticMasks[model];
        if ((mask & bit) != 0u)
            return;

        mask |= bit;
        ++DiagnosticMessages;

        char message[512] = { 0 };
        sprintf_s(message,
                  "candidate %.31s: bones=%d actions=%d mesh=%d flags=0x%08X texture=%d uv=(%.3f,%.3f,%.3f) -> %s",
                  model->Name,
                  static_cast<int>(GetPaletteBoneCount(command)),
                  static_cast<int>(model->NumActions),
                  command.m_IndexMesh,
                  static_cast<unsigned int>(command.m_FlagRender),
                  command.m_TextureID,
                  command.m_meshUV.x,
                  command.m_meshUV.y,
                  command.m_meshUV.z,
                  reason);
        ModernLog(message);

        if (DiagnosticMessages == DiagnosticLimit)
            ModernLog("diagnostic limit reached; further candidate fallback messages suppressed");
    }

    void LogDrawFailure(BMD* model,
                        const OGL330MODEL::RenderMeshVAO& command,
                        const char* reason)
    {
        if (!Diagnostics || reason == NULL || DrawFailureMessages >= DrawFailureLimit)
            return;

        ++DrawFailureMessages;
        char message[512] = { 0 };
        sprintf_s(message,
                  "isolated modern draw failed: %.31s mesh=%d paletteBones=%u flags=0x%08X -> %s",
                  (model != NULL && model->Name[0] != '\0') ? model->Name : "<none>",
                  command.m_IndexMesh,
                  GetPaletteBoneCount(command),
                  static_cast<unsigned int>(command.m_FlagRender),
                  reason);
        ModernLog(message);
    }

    bool IsEligible(const OGL330MODEL::RenderMeshVAO& command, bool logReason)
    {
        BMD* model = command.m_OldBMD;
        if (model == NULL)
            return false;

        const std::uint32_t boneCount = GetPaletteBoneCount(command);
        if (boneCount <= 1)
        {
            if (logReason)
                LogDiagnostic(model, command, DiagnosticBasic, "fallback: no captured skeletal palette or <= 1 bone");
            return false;
        }

        const MaterialMode material = ClassifyMaterial(command);
        if (!IsMaterialEncodingSupported(command, material))
        {
            if (logReason)
            {
                if (material == MaterialMode::Unsupported)
                    LogDiagnostic(model, command, DiagnosticMaterial,
                                  "material: unsupported flags (modern parity currently texture/texture-bright + Matrix4x4 shadow-texture/color/bright-color/Chrome01/Chrome04/Chrome05/Metal bright)");
                else
                    LogDiagnostic(model, command, DiagnosticMaterial,
                                  "material: unsupported Blend/stream UV encoding");
            }
            return false;
        }

        if (material != MaterialMode::Color &&
            material != MaterialMode::BrightColor &&
            command.m_TextureID < 0)
        {
            if (logReason)
                LogDiagnostic(model, command, DiagnosticBasic, "fallback: invalid texture id");
            return false;
        }

        if (boneCount < static_cast<std::uint32_t>(MinBones))
        {
            if (logReason)
                LogDiagnostic(model, command, DiagnosticFilter, "filter: below MinBones");
            return false;
        }

        if (!AtlasMode && model->NumActions < MinActions)
        {
            if (logReason)
                LogDiagnostic(model, command, DiagnosticFilter, "filter: below MinActions");
            return false;
        }

        if (TargetName[0] != '\0' && std::strstr(model->Name, TargetName) == NULL)
        {
            if (logReason)
                LogDiagnostic(model, command, DiagnosticFilter, "filter: TargetName mismatch");
            return false;
        }

        if (!AtlasMode && SelectedModel != NULL && SelectedModel != model)
            return false;

        return true;
    }

    ModernMeshGpu* GetMesh(BMD* model, int meshIndex)
    {
        MeshKey key = { model, meshIndex };
        std::unordered_map<MeshKey, std::unique_ptr<ModernMeshGpu>, MeshKeyHash>::iterator found = Meshes.find(key);
        if (found != Meshes.end())
            return found->second.get();

        if (model == NULL || meshIndex < 0 || meshIndex >= model->NumMeshs)
            return NULL;

        BMDModernMeshData cpuMesh;
        if (!BuildBMDModernMesh(model->Meshs[meshIndex], cpuMesh) || cpuMesh.Empty())
            return NULL;

        std::unique_ptr<ModernMeshGpu> gpuMesh(new ModernMeshGpu());
        if (!gpuMesh->Vao.Upload(cpuMesh))
            return NULL;

        ModernMeshGpu* result = gpuMesh.get();
        Meshes.insert(std::make_pair(key, std::move(gpuMesh)));
        return result;
    }
};

BMDModernRuntime& BMDModernRuntime::Instance()
{
    static BMDModernRuntime* instance = new BMDModernRuntime();
    return *instance;
}

BMDModernRuntime::BMDModernRuntime()
    : m_Impl(new Impl())
{
}

BMDModernRuntime::~BMDModernRuntime()
{
    delete m_Impl;
    m_Impl = NULL;
}

bool BMDModernRuntime::IsEnabled() const
{
    return m_Impl != NULL && m_Impl->Enabled;
}

bool BMDModernRuntime::IsBatchPrepared() const
{
    return m_Impl != NULL && m_Impl->Enabled &&
           m_Impl->BatchPrepared && m_Impl->Program != 0;
}

bool BMDModernRuntime::IsMaterialCompatible(const OGL330MODEL::RenderMeshVAO& command) const
{
    if (m_Impl == NULL || !m_Impl->Enabled)
        return false;

    const Impl::MaterialMode material = m_Impl->ClassifyMaterial(command);
    return m_Impl->IsMaterialEncodingSupported(command, material);
}

bool BMDModernRuntime::PrepareBatch(const OGL330MODEL::MeshVAO& commands)
{
    if (m_Impl == NULL || !m_Impl->Enabled || commands.empty() || !m_Impl->EnsureProgram())
        return false;

    m_Impl->BatchPrepared = false;
    m_Impl->Atlas.BeginFrame();

    BMD* firstLightCandidate = m_Impl->SelectedModel;

    for (OGL330MODEL::MeshVAO::const_iterator iter = commands.begin();
         iter != commands.end(); ++iter)
    {
        const OGL330MODEL::RenderMeshVAO& command = *iter;
        BMD* model = command.m_OldBMD;
        if (!BMDModernAllowModernForCommand(command.m_Owner))
        {
            if (model != NULL && m_Impl->Diagnostics)
            {
                static unsigned int ownerSkipLogs = 0;
                if (ownerSkipLogs < 4u)
                {
                    ++ownerSkipLogs;
                    char message[320] = { 0 };
                    sprintf_s(message,
                              "remote-rollout isolation: command owner is not selected remote; leaving legacy model=%.31s",
                              model->Name);
                    ModernLog(message);
                }
            }
            continue;
        }
        if (!m_Impl->IsEligible(command, true))
            continue;

        if (!m_Impl->AtlasMode)
        {
            if (firstLightCandidate == NULL)
                firstLightCandidate = model;
            if (firstLightCandidate != model)
                continue;
        }

        const void* poseKey = command.m_BonePalette.get();
        if (m_Impl->Atlas.Find(poseKey) == NULL &&
            m_Impl->Atlas.GetStats().PoseCount >= static_cast<std::uint32_t>(m_Impl->MaxAtlasPoses))
        {
            m_Impl->LogDiagnostic(model, command, Impl::DiagnosticAtlas, "atlas: AtlasMaxPoses reached");
            continue;
        }

        const std::uint32_t boneCount = GetPaletteBoneCount(command);
        const float* paletteData = command.m_BonePalette->data();
        std::vector<float> modernPalette;
        if (m_Impl->Mode == Impl::ProgramMode::Generated)
        {
            if (!BuildModernSkeletonPalette(command, modernPalette))
            {
                m_Impl->LogDiagnostic(model, command, Impl::DiagnosticSkeleton,
                                      "skeleton: immutable command transform could not be separated from captured legacy palette");
                continue;
            }
            paletteData = modernPalette.data();
        }

        const BMDModernSkeletonSubmission submission = m_Impl->Atlas.Stage(
            poseKey,
            paletteData,
            boneCount,
            1.0f);
        if (!submission.Success)
        {
            m_Impl->LogDiagnostic(
                model,
                command,
                Impl::DiagnosticSkeleton,
                m_Impl->MatrixSkeleton
                    ? "skeleton: captured palette rejected by Matrix4x4 encoder"
                    : "skeleton: captured palette rejected by QPS encoder");
        }
    }

    const BMDModernSkeletonAtlas::Stats& stats = m_Impl->Atlas.GetStats();
    if (stats.PoseCount == 0)
        return false;

    if (!m_Impl->SkeletonTexture.Upload(m_Impl->Atlas.GetBuffer()))
    {
        ModernLog("skeleton atlas BonesTexture upload failed; legacy fallback for batch");
        return false;
    }

    m_Impl->BatchPrepared = true;

    if (!m_Impl->TransformSnapshotLogged && m_Impl->Mode == Impl::ProgramMode::Generated)
    {
        m_Impl->TransformSnapshotLogged = true;
        ModernLog("generated-shader transform split uses immutable RenderMeshVAO snapshots; mutable BMD BodyScale/BodyOrigin are not read during flush");
    }

    if (!m_Impl->MaterialProgramsLogged && m_Impl->MatrixSkeleton &&
        m_Impl->ShadowTextureProgram != 0 &&
        m_Impl->ColorProgram != 0 && m_Impl->ChromeProgram != 0 &&
        m_Impl->Chrome04Program != 0 && m_Impl->Chrome05Program != 0 &&
        m_Impl->MetalProgram != 0)
    {
        m_Impl->MaterialProgramsLogged = true;
        ModernLog("material parity rollout active: COLOR (0x1), SHADOWMAP|TEXTURE (0x22), BRIGHT (0x40), TEXTURE|BRIGHT (0x42), CHROME|BRIGHT (0x44), METAL|BRIGHT (0x48), CHROME4|BRIGHT (0x1040), CHROME5|TEXTURE|BRIGHT (0x4042) use Matrix4x4 ModernBMD programs; texture family accepts base and BlendMesh UV offsets");
    }

    if (!m_Impl->AtlasUploadLogged)
    {
        m_Impl->AtlasUploadLogged = true;
        const char* firstModelName = "<none>";
        for (OGL330MODEL::MeshVAO::const_iterator iter = commands.begin();
             iter != commands.end(); ++iter)
        {
            if (iter->m_OldBMD != NULL && iter->m_OldBMD->Name[0] != '\0')
            {
                firstModelName = iter->m_OldBMD->Name;
                break;
            }
        }
        char message[384] = { 0 };
        sprintf_s(message,
                  "skeleton atlas uploaded once for batch: model=%.31s poses=%u bones=%u reused=%u commands=%u shader=%s encoding=%s texelsPerBone=%u",
                  firstModelName,
                  stats.PoseCount,
                  stats.BoneCount,
                  stats.ReusedPoseCount,
                  static_cast<unsigned int>(commands.size()),
                  m_Impl->Mode == Impl::ProgramMode::Generated ? "generated" : "experimental",
                  m_Impl->EncodingName(),
                  m_Impl->Atlas.GetBuffer().GetTexelsPerBone());
        ModernLog(message);
    }

    if (!m_Impl->MultiPoseLogged && stats.PoseCount > 1)
    {
        m_Impl->MultiPoseLogged = true;
        char message[256] = { 0 };
        sprintf_s(message,
                  "multi-pose skeleton atlas observed: poses=%u bones=%u (single BonesTexture upload, encoding=%s)",
                  stats.PoseCount,
                  stats.BoneCount,
                  m_Impl->EncodingName());
        ModernLog(message);
    }

    return true;
}

void BMDModernRuntime::FinishBatch()
{
    if (m_Impl != NULL)
    {
        m_Impl->Bindings.Unbind();
        m_Impl->GlobalConstants.Unbind();
        m_Impl->BatchPrepared = false;
    }
}

bool BMDModernRuntime::TryRender(const OGL330MODEL::RenderMeshVAO& command)
{
    if (m_Impl == NULL || !m_Impl->Enabled)
        return false;

    // Test immutable owner/material eligibility first. Unsupported legacy
    // overlays should silently use the legacy renderer and must not consume the
    // draw-failure budget merely because this batch was intentionally skipped.
    if (!BMDModernAllowModernForCommand(command.m_Owner) ||
        !m_Impl->IsEligible(command, true))
        return false;

    if (!m_Impl->BatchPrepared || m_Impl->Program == 0)
    {
        m_Impl->LogDrawFailure(command.m_OldBMD, command,
                               !m_Impl->BatchPrepared
                                   ? "draw: batch not prepared (coherence, empty atlas, or PrepareBatch failure)"
                                   : "draw: modern shader program unavailable");
        return false;
    }

    const Impl::MaterialMode material = m_Impl->ClassifyMaterial(command);
    const GLuint drawProgram = m_Impl->ProgramForMaterial(material);
    if (drawProgram == 0)
    {
        m_Impl->LogDrawFailure(command.m_OldBMD, command,
                               "draw: material-specific modern shader unavailable");
        return false;
    }

    BMD* model = command.m_OldBMD;
    const void* poseKey = command.m_BonePalette.get();
    const BMDModernSkeletonSubmission* submission = m_Impl->Atlas.Find(poseKey);
    if (submission == NULL || !submission->Success)
    {
        m_Impl->LogDrawFailure(model, command, "draw: pose missing from prepared skeleton atlas");
        return false;
    }

    ModernMeshGpu* mesh = m_Impl->GetMesh(model, command.m_IndexMesh);
    if (mesh == NULL || !mesh->Vao.IsValid())
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticMesh,
                              "mesh: modern CPU mesh/VAO creation failed");
        m_Impl->LogDrawFailure(model, command, "draw: modern CPU mesh/VAO creation failed");
        return false;
    }

    const bool useCommandTransform = m_Impl->Mode == Impl::ProgramMode::Generated;
    BMDModernInstanceParams params;
    params.Translate = useCommandTransform && command.m_ModernTranslate;
    params.BodyScale = params.Translate ? command.m_ModernBodyScale : 1.0f;
    params.BodyOrigin[0] = params.Translate ? command.m_ModernBodyOrigin.x : 0.0f;
    params.BodyOrigin[1] = params.Translate ? command.m_ModernBodyOrigin.y : 0.0f;
    params.BodyOrigin[2] = params.Translate ? command.m_ModernBodyOrigin.z : 0.0f;
    params.NormalOffset = 0.0f;
    params.EnableLight = command.m_isLight;

    const bool samplesMaterialTexture =
        material != Impl::MaterialMode::Color &&
        material != Impl::MaterialMode::BrightColor;
    const bool alphaTexture = samplesMaterialTexture && command.m_TextureID >= 0 &&
                              Bitmaps[command.m_TextureID].Components == 4;
    const bool additiveMaterial =
        (command.m_FlagRender & (RENDER_BRIGHT | RENDER_DARK | RENDER_LIGHTMAP)) != 0;
    params.MinAlpha = (!additiveMaterial && (command.m_isAlpha < 0.99f || alphaTexture))
        ? 0.25f
        : 0.0f;
    params.BlendUV[0] = command.m_meshUV.x;
    params.BlendUV[1] = command.m_meshUV.y;
    params.EnableShadow = false;
    params.ShadowHeight = 0.0f;
    params.BodyLight[0] = command.m_bodyLight.x;
    params.BodyLight[1] = command.m_bodyLight.y;
    params.BodyLight[2] = command.m_bodyLight.z;
    params.BodyLight[3] = command.m_bodyLight.w;
    params.ShadowLight[0] = 0.0f;
    params.ShadowLight[1] = 0.0f;
    params.ShadowLight[2] = 0.0f;
    params.ShadowLight[3] = 0.0f;
    params.BoneIndex = submission->BoneIndex;

    const BMDModernInstance instance = BMDModernInstanceBuilder::Build(params);
    if (!m_Impl->InstanceBuffer.UploadAndAttach(mesh->Vao.GetVertexArray(), &instance, 1))
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticInstance,
                              "instance: upload/attribute attach failed");
        m_Impl->LogDrawFailure(model, command, "draw: instance upload/attribute attach failed");
        return false;
    }

    float projection[16];
    float view[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glGetFloatv(GL_MODELVIEW_MATRIX, view);

    // OpenGLBMDModernBindings stores sampler locations for one linked program.
    // Reconfigure only on a material-program transition, preserving the cheap
    // base-texture path for repeated meshes.
    if (m_Impl->Bindings.GetProgram() != drawProgram &&
        !m_Impl->Bindings.ConfigureProgram(drawProgram))
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticBindings,
                              "bindings: material program sampler configuration failed");
        m_Impl->LogDrawFailure(model, command,
                               "draw: material program sampler configuration failed");
        return false;
    }

    GLint previousProgram = 0;
    GLint previousVertexArray = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVertexArray);
    glUseProgram(drawProgram);

    if (m_Impl->Mode == Impl::ProgramMode::Generated)
    {
        Render::ShaderGlobalConstants constants;
        BuildGeneratedGlobals(command, projection, view, constants);
        if (!m_Impl->GlobalConstants.Upload(constants))
        {
            m_Impl->LogDiagnostic(model, command, Impl::DiagnosticGlobals,
                                  "globals: GlobalConstants upload failed");
            m_Impl->LogDrawFailure(model, command, "draw: GlobalConstants upload failed");
            glUseProgram(static_cast<GLuint>(previousProgram));
            return false;
        }
        m_Impl->GlobalConstants.Bind();
    }
    else
    {
        glUniformMatrix4fv(m_Impl->ProjectionLocation, 1, GL_FALSE, projection);
        glUniformMatrix4fv(m_Impl->ViewLocation, 1, GL_FALSE, view);
    }

    const unsigned int materialTexture = samplesMaterialTexture
        ? Bitmaps[command.m_TextureID].TextureNumber
        : 0u;
    if (!m_Impl->Bindings.Bind(materialTexture, m_Impl->SkeletonTexture))
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticBindings,
                              "bindings: material/skeleton texture bind failed");
        m_Impl->LogDrawFailure(model, command, "draw: material/skeleton texture bind failed");
        m_Impl->GlobalConstants.Unbind();
        glUseProgram(static_cast<GLuint>(previousProgram));
        return false;
    }

    mesh->Vao.Bind();
    glDrawElementsInstanced(GL_TRIANGLES,
                            static_cast<GLsizei>(mesh->Vao.GetIndexCount()),
                            GL_UNSIGNED_INT,
                            NULL,
                            1);
    glBindVertexArray(static_cast<GLuint>(previousVertexArray));
    m_Impl->Bindings.Unbind();
    m_Impl->GlobalConstants.Unbind();
    glUseProgram(static_cast<GLuint>(previousProgram));

    ++m_Impl->SuccessfulDraws;

    const std::uint32_t boneCount = GetPaletteBoneCount(command);
    const std::uint64_t poseHash = HashSkeletonPose(command.m_BonePalette->data(), boneCount);

    if (m_Impl->FirstModernModel == NULL)
    {
        m_Impl->FirstModernModel = model;
        m_Impl->FirstPoseHash = poseHash;
        m_Impl->FirstPoseHashValid = poseHash != 0;
        if (!m_Impl->AtlasMode)
            m_Impl->SelectedModel = model;

        const char* materialName = "texture";
        if (material == Impl::MaterialMode::TextureBright)
            materialName = "texture-bright";
        else if (material == Impl::MaterialMode::ShadowTexture)
            materialName = "shadow-texture";
        else if (material == Impl::MaterialMode::Color)
            materialName = "color";
        else if (material == Impl::MaterialMode::BrightColor)
            materialName = "bright-color";
        else if (material == Impl::MaterialMode::Chrome01)
            materialName = "chrome01";
        else if (material == Impl::MaterialMode::Chrome04)
            materialName = "chrome04";
        else if (material == Impl::MaterialMode::Chrome05)
            materialName = "chrome05";
        else if (material == Impl::MaterialMode::Metal)
            materialName = "metal";

        char message[560] = { 0 };
        sprintf_s(message,
                  "atlas modern BMD selected: %.31s (bones=%u, actions=%d, mesh=%d, texture=%d, BaseBone=%u, action=%u, frame=%.3f, shader=%s, material=%s, encoding=%s, translate=%d, bodyScale=%.3f)",
                  model->Name,
                  boneCount,
                  static_cast<int>(model->NumActions),
                  command.m_IndexMesh,
                  command.m_TextureID,
                  submission->BoneIndex,
                  static_cast<unsigned int>(model->CurrentAction),
                  model->CurrentAnimation,
                  m_Impl->Mode == Impl::ProgramMode::Generated ? "generated" : "experimental",
                  materialName,
                  m_Impl->EncodingName(),
                  params.Translate ? 1 : 0,
                  params.BodyScale);
        ModernLog(message);
    }
    else if (!m_Impl->FirstPoseChangeLogged &&
             m_Impl->FirstModernModel == model &&
             m_Impl->FirstPoseHashValid && poseHash != 0 && poseHash != m_Impl->FirstPoseHash)
    {
        m_Impl->FirstPoseChangeLogged = true;
        char message[320] = { 0 };
        sprintf_s(message,
                  "atlas animated pose change observed: %.31s (draw=%llu, action=%u, frame=%.3f, encoding=%s)",
                  model->Name,
                  static_cast<unsigned long long>(m_Impl->SuccessfulDraws),
                  static_cast<unsigned int>(model->CurrentAction),
                  model->CurrentAnimation,
                  m_Impl->EncodingName());
        ModernLog(message);
    }

    return true;
}

#endif // jdk_shader_local330
