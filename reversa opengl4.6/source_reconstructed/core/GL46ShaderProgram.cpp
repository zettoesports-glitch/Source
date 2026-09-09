#include "../include/GL46ShaderProgram.h"
#include <array>
#include <vector>

namespace reverse_gl46 {

static bool ApiReady(const ShaderProgramApi& a) {
    return a.createShader && a.shaderSource && a.compileShader && a.getShaderiv &&
           a.getShaderInfoLog && a.deleteShader && a.createProgram &&
           a.attachShader && a.linkProgram && a.getProgramiv &&
           a.getProgramInfoLog && a.detachShader && a.deleteProgram;
}

static std::string ReadShaderLog(const ShaderProgramApi& api, GLUInt shader) {
    std::vector<char> buffer(16 * 1024, '\0');
    GLSizeI written = 0;
    api.getShaderInfoLog(shader, static_cast<GLSizeI>(buffer.size()), &written, buffer.data());
    if (written <= 0) return {};
    return std::string(buffer.data(), buffer.data() + written);
}

static std::string ReadProgramLog(const ShaderProgramApi& api, GLUInt program) {
    std::vector<char> buffer(16 * 1024, '\0');
    GLSizeI written = 0;
    api.getProgramInfoLog(program, static_cast<GLSizeI>(buffer.size()), &written, buffer.data());
    if (written <= 0) return {};
    return std::string(buffer.data(), buffer.data() + written);
}

static GLUInt Compile(const ShaderProgramApi& api, GLEnum type,
                      const ShaderSourceParts& s, std::string& log) {
    if (!s.version || !s.defineBlock || !s.body) return 0;
    GLUInt shader = api.createShader(type);
    if (!shader) return 0;
    const std::array<const char*, 3> pieces{s.version, s.defineBlock, s.body};
    api.shaderSource(shader, 3, pieces.data(), nullptr);
    api.compileShader(shader);
    GLInt ok = 0;
    api.getShaderiv(shader, kCompileStatus, &ok);
    log = ReadShaderLog(api, shader);
    if (!ok) {
        api.deleteShader(shader);
        return 0;
    }
    return shader;
}

bool BuildProgram3Part(const ShaderProgramApi& api,
                       const ShaderSourceParts& vertex,
                       const ShaderSourceParts& fragment,
                       ProgramBuildResult& out) {
    out = {};
    if (!ApiReady(api)) return false;

    const GLUInt vs = Compile(api, kVertexShader, vertex, out.vertexLog);
    if (!vs) return false;
    const GLUInt fs = Compile(api, kFragmentShader, fragment, out.fragmentLog);
    if (!fs) {
        api.deleteShader(vs);
        return false;
    }

    const GLUInt program = api.createProgram();
    if (!program) {
        api.deleteShader(vs);
        api.deleteShader(fs);
        return false;
    }
    api.attachShader(program, vs);
    api.attachShader(program, fs);
    api.linkProgram(program);
    GLInt ok = 0;
    api.getProgramiv(program, kLinkStatus, &ok);
    out.programLog = ReadProgramLog(api, program);
    api.detachShader(program, vs);
    api.detachShader(program, fs);
    api.deleteShader(vs);
    api.deleteShader(fs);
    if (!ok) {
        api.deleteProgram(program);
        return false;
    }
    out.program = program;
    return true;
}

} // namespace reverse_gl46
