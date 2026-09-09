// RECONSTRUÍDO a partir de Main.exe. Não é a source original.
// Evidência principal: VA 0x00470C30 e helper 0x00470D90.
#include <windows.h>
#include <GL/gl.h>
#include <cstdint>

namespace reverse_gl46 {

enum class RendererMode : int {
    LegacyGL2 = 0,
    OpenGL33Compatibility = 1,
    OpenGL46 = 2,
};

constexpr int WGL_CONTEXT_MAJOR_VERSION_ARB_ = 0x2091;
constexpr int WGL_CONTEXT_MINOR_VERSION_ARB_ = 0x2092;
constexpr int WGL_CONTEXT_FLAGS_ARB_         = 0x2094;
constexpr int WGL_CONTEXT_PROFILE_MASK_ARB_  = 0x9126;
constexpr int WGL_CONTEXT_CORE_PROFILE_BIT_ARB_ = 0x00000001;
constexpr int WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB_ = 0x00000002;

using PFNWGLCREATECONTEXTATTRIBSARBPROC_RE = HGLRC (WINAPI*)(HDC,HGLRC,const int*);

struct ContextAttempt {
    HGLRC context{};
    RendererMode requested{RendererMode::LegacyGL2};
    RendererMode selected{RendererMode::LegacyGL2};
    bool selectionChanged{};
    bool hasCreateContextAttribs{};
    bool tried46{};
    bool tried33{};
    bool requestedCore46{};
    DWORD lastError{};
};

// Reconstrução de alto nível do helper em 0x00470D90.
static bool TryCreateModernContext(
    HDC dc,
    HGLRC previous,
    PFNWGLCREATECONTEXTATTRIBSARBPROC_RE createAttribs,
    int major,
    int minor,
    int profileMask,
    HGLRC* outContext,
    DWORD* outError)
{
    const int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB_, major,
        WGL_CONTEXT_MINOR_VERSION_ARB_, minor,
        WGL_CONTEXT_FLAGS_ARB_, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB_, profileMask,
        0
    };

    SetLastError(ERROR_SUCCESS);
    HGLRC candidate = createAttribs(dc, nullptr, attribs);
    if (!candidate) {
        if (outError) *outError = GetLastError();
        return false;
    }

    // O binário libera o current antes de ativar o contexto novo.
    wglMakeCurrent(nullptr, nullptr);
    if (!wglMakeCurrent(dc, candidate)) {
        if (outError) *outError = GetLastError();
        wglDeleteContext(candidate);
        if (previous) wglMakeCurrent(dc, previous);
        return false;
    }

    if (outContext) *outContext = candidate;
    if (outError) *outError = ERROR_SUCCESS;
    return true;
}

// Reconstrução de alto nível da rotina em 0x00470C30.
bool CreateRequestedContext(HDC dc, HGLRC legacyContext, RendererMode requested,
                            bool requestCore46, ContextAttempt& result)
{
    result = {};
    result.requested = requested;
    result.selected = RendererMode::LegacyGL2;
    result.requestedCore46 = requestCore46;

    if (!dc || !legacyContext || requested == RendererMode::LegacyGL2) {
        // O caminho legacy não precisa de wglCreateContextAttribsARB.
        result.context = legacyContext;
        result.selected = RendererMode::LegacyGL2;
        return true;
    }

    auto createAttribs = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC_RE>(
        wglGetProcAddress("wglCreateContextAttribsARB"));
    result.hasCreateContextAttribs = createAttribs != nullptr;
    if (!createAttribs) {
        result.context = legacyContext;
        result.selectionChanged = true;
        return true;
    }

    HGLRC modern{};
    DWORD error{};

    if (requested == RendererMode::OpenGL46) {
        result.tried46 = true;
        const int profile = requestCore46
            ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB_
            : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB_;
        if (TryCreateModernContext(dc, legacyContext, createAttribs, 4, 6, profile, &modern, &error)) {
            result.context = modern;
            result.selected = RendererMode::OpenGL46;
            return true;
        }
        result.lastError = error;
    }

    result.tried33 = true;
    if (TryCreateModernContext(dc, legacyContext, createAttribs, 3, 3,
                               WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB_, &modern, &error)) {
        result.context = modern;
        result.selected = RendererMode::OpenGL33Compatibility;
        result.selectionChanged = requested != result.selected;
        return true;
    }

    result.lastError = error;
    result.context = legacyContext;
    result.selected = RendererMode::LegacyGL2;
    result.selectionChanged = requested != result.selected;
    return true;
}

} // namespace reverse_gl46
