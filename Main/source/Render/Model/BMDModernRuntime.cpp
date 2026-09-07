#include "stdafx.h"
#include "BMDModernRuntime.h"

#if jdk_shader_local330

#include "BMDModernInstanceBuilder.h"
#include "BMDModernMesh.h"
#include "BMDModernSkeletonAtlas.h"
#include "../OpenGL/OpenGLBMDModernBindings.h"
#include "../OpenGL/OpenGLBMDModernInstanceBuffer.h"
#include "../OpenGL/OpenGLBMDModernVAO.h"
#include "../OpenGL/OpenGLSkeletonTexture.h"
#include "ZzzBMD.h"
#include "ZzzTexture.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

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

    static GLuint CompileStage(GLenum type, const std::string& source, const char* label)
    {
        if (source.empty())
            return 0;

        const char* text = source.c_str();
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
        {
            ModernLog("experimental shader files not found; using legacy fallback");
            return 0;
        }

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
        DiagnosticAtlas = 1u << 7
    };

    bool Enabled;
    bool AtlasMode;
    bool Diagnostics;
    int DiagnosticLimit;
    int DiagnosticMessages;
    int MinBones;
    int MinActions;
    int MaxAtlasPoses;
    char TargetName[64];

    bool ProgramAttempted;
    GLuint Program;
    GLint ProjectionLocation;
    GLint ViewLocation;

    bool BatchPrepared;
    bool AtlasUploadLogged;
    bool MultiPoseLogged;
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
    std::unordered_map<MeshKey, std::unique_ptr<ModernMeshGpu>, MeshKeyHash> Meshes;
    std::unordered_map<BMD*, unsigned int> DiagnosticMasks;

    Impl()
        : Enabled(GetPrivateProfileIntA("ModernRenderer", "ExperimentalBMD", 0,
                                        ".\\Data\\Custom\\config.ini") != 0)
        , AtlasMode(GetPrivateProfileIntA("ModernRenderer", "AtlasMode", 1,
                                          ".\\Data\\Custom\\config.ini") != 0)
        , Diagnostics(GetPrivateProfileIntA("ModernRenderer", "Diagnostics", 1,
                                            ".\\Data\\Custom\\config.ini") != 0)
        , DiagnosticLimit(GetPrivateProfileIntA("ModernRenderer", "DiagnosticLimit", 24,
                                                ".\\Data\\Custom\\config.ini"))
        , DiagnosticMessages(0)
        , MinBones(GetPrivateProfileIntA("ModernRenderer", "MinBones", 20,
                                         ".\\Data\\Custom\\config.ini"))
        , MinActions(GetPrivateProfileIntA("ModernRenderer", "MinActions", 2,
                                           ".\\Data\\Custom\\config.ini"))
        , MaxAtlasPoses(GetPrivateProfileIntA("ModernRenderer", "AtlasMaxPoses", 32,
                                              ".\\Data\\Custom\\config.ini"))
        , ProgramAttempted(false)
        , Program(0)
        , ProjectionLocation(-1)
        , ViewLocation(-1)
        , BatchPrepared(false)
        , AtlasUploadLogged(false)
        , MultiPoseLogged(false)
        , SelectedModel(NULL)
        , FirstModernModel(NULL)
        , FirstPoseHashValid(false)
        , FirstPoseChangeLogged(false)
        , FirstPoseHash(0)
        , SuccessfulDraws(0)
        , Atlas(SkeletonBuffer::StorageMode::QuaternionPositionScale)
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
            char message[384] = { 0 };
            sprintf_s(message,
                      "ExperimentalBMD=1; atlas runtime enabled (AtlasMode=%d, MinBones=%d, MinActions=%d, AtlasMaxPoses=%d, TargetName=%s, Diagnostics=%d)",
                      AtlasMode ? 1 : 0,
                      MinBones,
                      MinActions,
                      MaxAtlasPoses,
                      TargetName[0] != '\0' ? TargetName : "<any>",
                      Diagnostics ? 1 : 0);
            ModernLog(message);

            if (AtlasMode && MinActions > 1)
                ModernLog("AtlasMode uses captured bone palettes; MinActions is ignored so animated player equipment with one local action can participate");
        }
        else
        {
            ModernLog("ExperimentalBMD=0; legacy renderer only");
        }
    }

    bool EnsureProgram()
    {
        if (Program != 0)
            return true;
        if (ProgramAttempted)
            return false;

        ProgramAttempted = true;
        Program = LoadProgram("Data\\Effect\\Modern\\BMDExperimental.vs",
                              "Data\\Effect\\Modern\\BMDExperimental.fs");
        if (Program == 0)
            return false;

        if (!Bindings.ConfigureProgram(Program))
        {
            ModernLog("sampler contract mismatch; using legacy fallback");
            glDeleteProgram(Program);
            Program = 0;
            return false;
        }

        ProjectionLocation = glGetUniformLocation(Program, "uProj");
        ViewLocation = glGetUniformLocation(Program, "uView");
        if (ProjectionLocation < 0 || ViewLocation < 0)
        {
            ModernLog("camera uniforms missing; using legacy fallback");
            glDeleteProgram(Program);
            Program = 0;
            return false;
        }

        ModernLog("experimental shader program ready");
        return true;
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

        if (command.m_TextureID < 0)
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

        // In atlas mode the immutable palette is the animation source. Player
        // equipment often reports one local Action while its captured matrices
        // still change every frame with the owning character skeleton.
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

        const int allowedFlags = RENDER_TEXTURE | RENDER_NODEPTH;
        if ((command.m_FlagRender & RENDER_TEXTURE) == 0)
        {
            if (logReason)
                LogDiagnostic(model, command, DiagnosticMaterial, "material: not RENDER_TEXTURE");
            return false;
        }

        if ((command.m_FlagRender & ~allowedFlags) != 0)
        {
            if (logReason)
                LogDiagnostic(model, command, DiagnosticMaterial, "material: unsupported render flags");
            return false;
        }

        if (command.m_meshUV.z != 0.0f || command.m_meshUV.x != 0.0f || command.m_meshUV.y != 0.0f)
        {
            if (logReason)
                LogDiagnostic(model, command, DiagnosticMaterial, "material: Blend/stream UV path unsupported");
            return false;
        }

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
    // Process-lifetime on purpose: do not destroy GL objects after WGL teardown.
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
        const BMDModernSkeletonSubmission submission = m_Impl->Atlas.Stage(
            poseKey,
            command.m_BonePalette->data(),
            boneCount,
            1.0f);
        if (!submission.Success)
        {
            m_Impl->LogDiagnostic(model, command, Impl::DiagnosticSkeleton,
                                  "skeleton: captured palette rejected by QPS encoder");
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

    if (!m_Impl->AtlasUploadLogged)
    {
        m_Impl->AtlasUploadLogged = true;
        char message[256] = { 0 };
        sprintf_s(message,
                  "skeleton atlas uploaded once for batch: poses=%u bones=%u reused=%u commands=%u",
                  stats.PoseCount,
                  stats.BoneCount,
                  stats.ReusedPoseCount,
                  static_cast<unsigned int>(commands.size()));
        ModernLog(message);
    }

    if (!m_Impl->MultiPoseLogged && stats.PoseCount > 1)
    {
        m_Impl->MultiPoseLogged = true;
        char message[224] = { 0 };
        sprintf_s(message,
                  "multi-pose skeleton atlas observed: poses=%u bones=%u (single BonesTexture upload)",
                  stats.PoseCount,
                  stats.BoneCount);
        ModernLog(message);
    }

    return true;
}

void BMDModernRuntime::FinishBatch()
{
    if (m_Impl != NULL)
        m_Impl->BatchPrepared = false;
}

bool BMDModernRuntime::TryRender(const OGL330MODEL::RenderMeshVAO& command)
{
    if (m_Impl == NULL || !m_Impl->Enabled || !m_Impl->BatchPrepared ||
        m_Impl->Program == 0 || !m_Impl->IsEligible(command, true))
    {
        return false;
    }

    BMD* model = command.m_OldBMD;
    const void* poseKey = command.m_BonePalette.get();
    const BMDModernSkeletonSubmission* submission = m_Impl->Atlas.Find(poseKey);
    if (submission == NULL || !submission->Success)
        return false;

    ModernMeshGpu* mesh = m_Impl->GetMesh(model, command.m_IndexMesh);
    if (mesh == NULL || !mesh->Vao.IsValid())
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticMesh,
                              "mesh: modern CPU mesh/VAO creation failed");
        return false;
    }

    BMDModernInstanceParams params;

    // m_BonePalette is already the immutable legacy-final affine palette:
    // requestScale, BodyScale and BodyOrigin are baked into the 3x4 matrices.
    // Keep instance transform at identity to avoid applying them twice.
    params.Translate = false;
    params.BodyScale = 1.0f;
    params.NormalOffset = 0.0f;
    params.EnableLight = command.m_isLight;

    const bool alphaTexture = Bitmaps[command.m_TextureID].Components == 4;
    params.MinAlpha = (command.m_isAlpha < 0.99f || alphaTexture) ? 0.25f : 0.0f;
    params.BlendUV[0] = 0.0f;
    params.BlendUV[1] = 0.0f;
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
        return false;
    }

    float projection[16];
    float view[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glGetFloatv(GL_MODELVIEW_MATRIX, view);

    glUseProgram(m_Impl->Program);
    glUniformMatrix4fv(m_Impl->ProjectionLocation, 1, GL_FALSE, projection);
    glUniformMatrix4fv(m_Impl->ViewLocation, 1, GL_FALSE, view);

    if (!m_Impl->Bindings.Bind(Bitmaps[command.m_TextureID].TextureNumber,
                               m_Impl->SkeletonTexture))
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticBindings,
                              "bindings: material/skeleton texture bind failed");
        glUseProgram(0);
        return false;
    }

    mesh->Vao.Bind();
    glDrawElementsInstanced(GL_TRIANGLES,
                            static_cast<GLsizei>(mesh->Vao.GetIndexCount()),
                            GL_UNSIGNED_INT,
                            NULL,
                            1);
    glBindVertexArray(0);
    glUseProgram(0);

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

        char message[352] = { 0 };
        sprintf_s(message,
                  "atlas modern BMD selected: %.31s (bones=%u, actions=%d, mesh=%d, texture=%d, BaseBone=%u, action=%u, frame=%.3f)",
                  model->Name,
                  boneCount,
                  static_cast<int>(model->NumActions),
                  command.m_IndexMesh,
                  command.m_TextureID,
                  submission->BoneIndex,
                  static_cast<unsigned int>(model->CurrentAction),
                  model->CurrentAnimation);
        ModernLog(message);
    }
    else if (!m_Impl->FirstPoseChangeLogged &&
             m_Impl->FirstModernModel == model &&
             m_Impl->FirstPoseHashValid && poseHash != 0 && poseHash != m_Impl->FirstPoseHash)
    {
        m_Impl->FirstPoseChangeLogged = true;
        char message[288] = { 0 };
        sprintf_s(message,
                  "atlas animated pose change observed: %.31s (draw=%llu, action=%u, frame=%.3f)",
                  model->Name,
                  static_cast<unsigned long long>(m_Impl->SuccessfulDraws),
                  static_cast<unsigned int>(model->CurrentAction),
                  model->CurrentAnimation);
        ModernLog(message);
    }

    return true;
}

#endif // jdk_shader_local330
