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

        // Keep a tiny persistent first-light trace in the client Data folder.
        // This lets runtime validation distinguish a real modern draw from a
        // visually identical legacy fallback without requiring a debugger.
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
    bool Enabled;
    bool ProgramAttempted;
    GLuint Program;
    GLint ProjectionLocation;
    GLint ViewLocation;
    BMD* SelectedModel;

    SkeletonBuffer Skeleton;
    OpenGLSkeletonTexture SkeletonTexture;
    OpenGLBMDModernBindings Bindings;
    OpenGLBMDModernInstanceBuffer InstanceBuffer;
    std::unordered_map<MeshKey, std::unique_ptr<ModernMeshGpu>, MeshKeyHash> Meshes;

    Impl()
        : Enabled(GetPrivateProfileIntA("ModernRenderer", "ExperimentalBMD", 0,
                                        ".\\Data\\Custom\\config.ini") != 0)
        , ProgramAttempted(false)
        , Program(0)
        , ProjectionLocation(-1)
        , ViewLocation(-1)
        , SelectedModel(NULL)
        , Skeleton(SkeletonBuffer::StorageMode::QuaternionPositionScale)
    {
        ModernLog(Enabled
            ? "ExperimentalBMD=1; first-light runtime enabled"
            : "ExperimentalBMD=0; legacy renderer only");
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
    if (model == NULL || model->NumBones <= 1 || model->m_pLastBoneMatrix == NULL ||
        command.m_TextureID < 0)
        return false;

    // First-light supports only the ordinary texture path. BlendMesh exposes
    // meshUV.z=1 in the legacy frontend, while chrome/metal/oil/shadow/etc. all
    // carry flags outside this small allow-list and therefore fall back.
    const int allowedFlags = RENDER_TEXTURE | RENDER_NODEPTH;
    if ((command.m_FlagRender & RENDER_TEXTURE) == 0 ||
        (command.m_FlagRender & ~allowedFlags) != 0 ||
        command.m_meshUV.z != 0.0f || command.m_meshUV.x != 0.0f || command.m_meshUV.y != 0.0f)
        return false;

    if (m_Impl->SelectedModel != NULL && m_Impl->SelectedModel != model)
        return false;

    ModernMeshGpu* mesh = m_Impl->GetMesh(model, command.m_IndexMesh);
    if (mesh == NULL || !mesh->Vao.IsValid())
        return false;

    // Validation-stage policy: one skeleton upload per experimental draw. This
    // avoids changing the proven legacy batching contract. Once visuals match,
    // this moves to one atlas upload per FlushAllMesh().
    m_Impl->Skeleton.BeginFrame();
    const BMDModernSkeletonSubmission submission = BMDModernSkeletonPose::Stage(
        m_Impl->Skeleton,
        &model->m_pLastBoneMatrix[0][0][0],
        static_cast<std::uint32_t>(model->NumBones),
        model->m_fRequestScale);
    if (!submission.Success || !m_Impl->SkeletonTexture.Upload(m_Impl->Skeleton))
        return false;

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
        return false;

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
        m_Impl->SelectedModel = model;
        char message[160] = { 0 };
        sprintf_s(message, "first modern BMD selected: %s (%d bones)",
                  model->Name, static_cast<int>(model->NumBones));
        ModernLog(message);
    }

    return true;
}

#endif // jdk_shader_local330
