#include "stdafx.h"
#include "BMDModernRuntime.h"

#if jdk_shader_local330

#include "BMDModernInstanceBuilder.h"
#include "BMDModernMesh.h"
#include "BMDModernSkeletonPose.h"
#include "SkeletonBuffer.h"
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

        // Persistent validation trace. This distinguishes a real modern draw
        // from a visually identical legacy fallback without requiring a debugger.
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
            std::string message = std::string(label ? label : "shader") + ": " + log;
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
            std::string message = std::string("program link: ") + log;
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

        // FNV-1a over the exact affine matrices. A different hash on a later
        // draw proves that a live animated pose reached the modern skeleton path.
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
        DiagnosticBindings = 1u << 6
    };

    bool Enabled;
    bool Diagnostics;
    int DiagnosticLimit;
    int DiagnosticMessages;
    int MinBones;
    int MinActions;
    char TargetName[64];
    bool ProgramAttempted;
    GLuint Program;
    GLint ProjectionLocation;
    GLint ViewLocation;
    BMD* SelectedModel;
    bool InitialPoseHashValid;
    bool PoseChangeLogged;
    std::uint64_t InitialPoseHash;
    std::uint64_t SuccessfulDraws;

    SkeletonBuffer Skeleton;
    OpenGLSkeletonTexture SkeletonTexture;
    OpenGLBMDModernBindings Bindings;
    OpenGLBMDModernInstanceBuffer InstanceBuffer;
    std::unordered_map<MeshKey, std::unique_ptr<ModernMeshGpu>, MeshKeyHash> Meshes;
    std::unordered_map<BMD*, unsigned int> DiagnosticMasks;

    Impl()
        : Enabled(GetPrivateProfileIntA("ModernRenderer", "ExperimentalBMD", 0,
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
        , ProgramAttempted(false)
        , Program(0)
        , ProjectionLocation(-1)
        , ViewLocation(-1)
        , SelectedModel(NULL)
        , InitialPoseHashValid(false)
        , PoseChangeLogged(false)
        , InitialPoseHash(0)
        , SuccessfulDraws(0)
        , Skeleton(SkeletonBuffer::StorageMode::QuaternionPositionScale)
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
        if (DiagnosticLimit < 1)
            DiagnosticLimit = 1;
        if (DiagnosticLimit > 128)
            DiagnosticLimit = 128;

        if (Enabled)
        {
            char message[320] = { 0 };
            sprintf_s(message,
                      "ExperimentalBMD=1; complex validation enabled (MinBones=%d, MinActions=%d, TargetName=%s, Diagnostics=%d, Limit=%d)",
                      MinBones,
                      MinActions,
                      TargetName[0] != '\0' ? TargetName : "<any>",
                      Diagnostics ? 1 : 0,
                      DiagnosticLimit);
            ModernLog(message);
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
                  static_cast<int>(model->NumBones),
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

bool BMDModernRuntime::TryRender(const OGL330MODEL::RenderMeshVAO& command)
{
    if (m_Impl == NULL || !m_Impl->Enabled || !m_Impl->EnsureProgram())
        return false;

    BMD* model = command.m_OldBMD;
    if (model == NULL)
        return false;

    if (model->NumBones <= 1)
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticBasic, "fallback: NumBones <= 1");
        return false;
    }
    if (model->m_pLastBoneMatrix == NULL)
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticBasic, "fallback: no last BoneTransform");
        return false;
    }
    if (command.m_TextureID < 0)
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticBasic, "fallback: invalid texture id");
        return false;
    }

    // Until one complex model is proven, ignore small/static BMDs. Once a model
    // succeeds it becomes the only selected asset for this process, keeping the
    // experiment narrow while all other BMDs continue through the legacy path.
    if (m_Impl->SelectedModel == NULL)
    {
        if (model->NumBones < m_Impl->MinBones)
        {
            m_Impl->LogDiagnostic(model, command, Impl::DiagnosticFilter, "filter: below MinBones");
            return false;
        }
        if (model->NumActions < m_Impl->MinActions)
        {
            m_Impl->LogDiagnostic(model, command, Impl::DiagnosticFilter, "filter: below MinActions");
            return false;
        }
        if (m_Impl->TargetName[0] != '\0' && std::strstr(model->Name, m_Impl->TargetName) == NULL)
        {
            m_Impl->LogDiagnostic(model, command, Impl::DiagnosticFilter, "filter: TargetName mismatch");
            return false;
        }
    }
    else if (m_Impl->SelectedModel != model)
    {
        return false;
    }

    // Complex-validation still supports only the ordinary texture path.
    const int allowedFlags = RENDER_TEXTURE | RENDER_NODEPTH;
    if ((command.m_FlagRender & RENDER_TEXTURE) == 0)
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticMaterial, "material: not RENDER_TEXTURE");
        return false;
    }
    if ((command.m_FlagRender & ~allowedFlags) != 0)
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticMaterial, "material: unsupported render flags");
        return false;
    }
    if (command.m_meshUV.z != 0.0f || command.m_meshUV.x != 0.0f || command.m_meshUV.y != 0.0f)
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticMaterial, "material: Blend/stream UV path unsupported");
        return false;
    }

    ModernMeshGpu* mesh = m_Impl->GetMesh(model, command.m_IndexMesh);
    if (mesh == NULL || !mesh->Vao.IsValid())
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticMesh, "mesh: modern CPU mesh/VAO creation failed");
        return false;
    }

    // Validation-stage policy: one skeleton upload per experimental draw. This
    // avoids changing the proven legacy batching contract. Once visuals match,
    // this moves to one atlas upload per FlushAllMesh().
    m_Impl->Skeleton.BeginFrame();
    const BMDModernSkeletonSubmission submission = BMDModernSkeletonPose::Stage(
        m_Impl->Skeleton,
        &model->m_pLastBoneMatrix[0][0][0],
        static_cast<std::uint32_t>(model->NumBones),
        model->m_fRequestScale);
    if (!submission.Success)
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticSkeleton, "skeleton: Stage/encoder rejected pose");
        return false;
    }
    if (!m_Impl->SkeletonTexture.Upload(m_Impl->Skeleton))
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticSkeleton, "skeleton: BonesTexture upload failed");
        return false;
    }

    BMDModernInstanceParams params;
    params.Translate = model->m_bLastTranslate;
    params.BodyScale = model->BodyScale;
    params.BodyOrigin[0] = model->BodyOrigin[0];
    params.BodyOrigin[1] = model->BodyOrigin[1];
    params.BodyOrigin[2] = model->BodyOrigin[2];
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
    params.BoneIndex = submission.BoneIndex;

    const BMDModernInstance instance = BMDModernInstanceBuilder::Build(params);
    if (!m_Impl->InstanceBuffer.UploadAndAttach(mesh->Vao.GetVertexArray(), &instance, 1))
    {
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticInstance, "instance: upload/attribute attach failed");
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
        m_Impl->LogDiagnostic(model, command, Impl::DiagnosticBindings, "bindings: material/skeleton texture bind failed");
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
    const std::uint64_t poseHash = HashSkeletonPose(
        &model->m_pLastBoneMatrix[0][0][0],
        static_cast<std::uint32_t>(model->NumBones));

    if (m_Impl->SelectedModel == NULL)
    {
        m_Impl->SelectedModel = model;
        m_Impl->InitialPoseHash = poseHash;
        m_Impl->InitialPoseHashValid = poseHash != 0;

        char message[320] = { 0 };
        sprintf_s(message,
                  "complex modern BMD selected: %.31s (bones=%d, actions=%d, mesh=%d, texture=%d, action=%u, frame=%.3f)",
                  model->Name,
                  static_cast<int>(model->NumBones),
                  static_cast<int>(model->NumActions),
                  command.m_IndexMesh,
                  command.m_TextureID,
                  static_cast<unsigned int>(model->CurrentAction),
                  model->CurrentAnimation);
        ModernLog(message);
    }
    else if (!m_Impl->PoseChangeLogged && m_Impl->InitialPoseHashValid &&
             poseHash != 0 && poseHash != m_Impl->InitialPoseHash)
    {
        m_Impl->PoseChangeLogged = true;
        char message[256] = { 0 };
        sprintf_s(message,
                  "animated pose change observed: %.31s (draw=%llu, action=%u, frame=%.3f)",
                  model->Name,
                  static_cast<unsigned long long>(m_Impl->SuccessfulDraws),
                  static_cast<unsigned int>(model->CurrentAction),
                  model->CurrentAnimation);
        ModernLog(message);
    }

    return true;
}

#endif // jdk_shader_local330
