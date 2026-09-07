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
            ModernLog("experimental shader files were not found; legacy fallback remains active");
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
            const size_t pointerHash = reinterpret_cast<size_t>(key.Model) >> 4;
            return pointerHash ^ (static_cast<size_t>(key.MeshIndex) * static_cast<size_t>(2654435761u));
        }
    };

    struct ModernMeshGpu
    {
        OpenGLBMDModernVAO Vao;
    };
}

struct BMDModernRuntime::Impl
{
    bool Enabled;
    bool FrameReady;
    bool ProgramAttempted;
    GLuint Program;
    GLint ProjectionLocation;
    GLint ViewLocation;
    BMD* SelectedModel;

    SkeletonBuffer Skeleton;
    OpenGLSkeletonTexture SkeletonTexture;
    OpenGLBMDModernBindings Bindings;
    OpenGLBMDModernInstanceBuffer InstanceBuffer;

    std::unordered_map<const std::vector<float>*, BMDModernSkeletonSubmission> Submissions;
    std::unordered_map<MeshKey, std::unique_ptr<ModernMeshGpu>, MeshKeyHash> Meshes;

    Impl()
        : Enabled(GetPrivateProfileIntA("ModernRenderer", "ExperimentalBMD", 0,
                                        ".\\Data\\Custom\\config.ini") != 0)
        , FrameReady(false)
        , ProgramAttempted(false)
        , Program(0)
        , ProjectionLocation(-1)
        , ViewLocation(-1)
        , SelectedModel(NULL)
        , Skeleton(SkeletonBuffer::StorageMode::QuaternionPositionScale)
    {
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
            ModernLog("sampler contract mismatch; legacy fallback remains active");
            glDeleteProgram(Program);
            Program = 0;
            return false;
        }

        ProjectionLocation = glGetUniformLocation(Program, "uProj");
        ViewLocation = glGetUniformLocation(Program, "uView");
        if (ProjectionLocation < 0 || ViewLocation < 0)
        {
            ModernLog("camera uniforms missing; legacy fallback remains active");
            glDeleteProgram(Program);
            Program = 0;
            return false;
        }

        ModernLog("experimental shader program ready");
        return true;
    }

    ModernMeshGpu* GetMesh(BMD* model, int meshIndex)
    {
        MeshKey key = { model, meshIndex };
        std::unordered_map<MeshKey, std::unique_ptr<ModernMeshGpu>, MeshKeyHash>::iterator it = Meshes.find(key);
        if (it != Meshes.end())
            return it->second.get();

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
    // Intentionally process-lifetime: OpenGL resources must not be destroyed by
    // static destructors after the WGL context has already been released.
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

void BMDModernRuntime::PrepareFrame(const OGL330MODEL::MeshVAO& commands)
{
    if (m_Impl == NULL)
        return;

    m_Impl->FrameReady = false;
    m_Impl->Submissions.clear();

    if (!m_Impl->Enabled || commands.empty() || !m_Impl->EnsureProgram())
        return;

    m_Impl->Skeleton.BeginFrame();

    for (OGL330MODEL::MeshVAO::const_iterator it = commands.begin(); it != commands.end(); ++it)
    {
        const OGL330MODEL::RenderMeshVAO& command = *it;
        if (command.m_OldBMD == NULL || !command.m_ModernBoneMatrices ||
            command.m_ModernBoneMatrices->empty() || command.m_OldBMD->NumBones <= 0)
            continue;

        const std::vector<float>* key = command.m_ModernBoneMatrices.get();
        if (m_Impl->Submissions.find(key) != m_Impl->Submissions.end())
            continue;

        const std::uint32_t boneCount = static_cast<std::uint32_t>(command.m_OldBMD->NumBones);
        if (command.m_ModernBoneMatrices->size() < static_cast<size_t>(boneCount) * 12u)
            continue;

        const BMDModernSkeletonSubmission submission = BMDModernSkeletonPose::Stage(
            m_Impl->Skeleton,
            command.m_ModernBoneMatrices->data(),
            boneCount,
            command.m_OldBMD->m_fRequestScale);

        if (submission.Success)
            m_Impl->Submissions.insert(std::make_pair(key, submission));
    }

    if (m_Impl->Submissions.empty())
        return;

    if (!m_Impl->SkeletonTexture.Upload(m_Impl->Skeleton))
    {
        ModernLog("BonesTexture upload failed; legacy fallback remains active");
        return;
    }

    m_Impl->FrameReady = true;
}

bool BMDModernRuntime::TryRender(const OGL330MODEL::RenderMeshVAO& command)
{
    if (m_Impl == NULL || !m_Impl->Enabled || !m_Impl->FrameReady || m_Impl->Program == 0)
        return false;

    if (command.m_OldBMD == NULL || command.m_OldBMD->NumBones <= 1 ||
        !command.m_ModernBoneMatrices || command.m_TextureID < 0)
        return false;

    // Phase 7 first-light is intentionally restricted to the ordinary Model
    // shader and texture-only render state. Every special material remains on
    // the proven legacy path until the base skinning draw is validated.
    if (command.m_Shader != OGL330MODEL::GetBaseModelShaderProgram())
        return false;

    const int allowedFlags = RENDER_TEXTURE | RENDER_NODEPTH;
    if ((command.m_FlagRender & RENDER_TEXTURE) == 0 ||
        (command.m_FlagRender & ~allowedFlags) != 0)
        return false;

    const std::vector<float>* submissionKey = command.m_ModernBoneMatrices.get();
    std::unordered_map<const std::vector<float>*, BMDModernSkeletonSubmission>::const_iterator submissionIt =
        m_Impl->Submissions.find(submissionKey);
    if (submissionIt == m_Impl->Submissions.end() || !submissionIt->second.Success)
        return false;

    if (m_Impl->SelectedModel != NULL && m_Impl->SelectedModel != command.m_OldBMD)
        return false;

    ModernMeshGpu* mesh = m_Impl->GetMesh(command.m_OldBMD, command.m_IndexMesh);
    if (mesh == NULL || !mesh->Vao.IsValid())
        return false;

    BMDModernInstanceParams params;
    params.Translate = command.m_ModernTranslate;
    params.BodyScale = command.m_OldBMD->BodyScale;
    params.BodyOrigin[0] = command.m_OldBMD->BodyOrigin[0];
    params.BodyOrigin[1] = command.m_OldBMD->BodyOrigin[1];
    params.BodyOrigin[2] = command.m_OldBMD->BodyOrigin[2];
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
    params.BoneIndex = submissionIt->second.BoneIndex;

    const BMDModernInstance instance = BMDModernInstanceBuilder::Build(params);
    if (!m_Impl->InstanceBuffer.UploadAndAttach(mesh->Vao.GetVertexArray(), &instance, 1))
        return false;

    float projection[16];
    float view[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glGetFloatv(GL_MODELVIEW_MATRIX, view);

    glUseProgram(m_Impl->Program);
    glUniformMatrix4fv(m_Impl->ProjectionLocation, 1, GL_FALSE, projection);
    glUniformMatrix4fv(m_Impl->ViewLocation, 1, GL_FALSE, view);

    const GLuint materialTexture = Bitmaps[command.m_TextureID].TextureNumber;
    if (!m_Impl->Bindings.Bind(materialTexture, m_Impl->SkeletonTexture))
    {
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

    if (m_Impl->SelectedModel == NULL)
    {
        m_Impl->SelectedModel = command.m_OldBMD;
        char message[160];
        sprintf_s(message, "first modern BMD selected: %s (%d bones)",
                  command.m_OldBMD->Name,
                  static_cast<int>(command.m_OldBMD->NumBones));
        ModernLog(message);
    }

    return true;
}

void BMDModernRuntime::EndFrame()
{
    if (m_Impl == NULL)
        return;

    m_Impl->FrameReady = false;
    m_Impl->Submissions.clear();
}

#endif // jdk_shader_local330
