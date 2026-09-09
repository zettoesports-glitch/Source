// RECONSTRUÍDO a partir de Main.exe. Não é a source original.
// Evidência principal: VA 0x00470C30 e helper 0x00470D90.
#include "../include/RenderDevice3D.h"
#ifdef _WIN32
#include <GL/gl.h>

namespace reverse_gl46 {

constexpr int WGL_CONTEXT_MAJOR_VERSION_ARB_ = 0x2091;
constexpr int WGL_CONTEXT_MINOR_VERSION_ARB_ = 0x2092;
constexpr int WGL_CONTEXT_FLAGS_ARB_         = 0x2094;
constexpr int WGL_CONTEXT_PROFILE_MASK_ARB_  = 0x9126;
constexpr int WGL_CONTEXT_CORE_PROFILE_BIT_ARB_ = 0x00000001;
constexpr int WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB_ = 0x00000002;

using PFNWGLCREATECONTEXTATTRIBSARBPROC_RE = HGLRC (WINAPI*)(HDC,HGLRC,const int*);

enum class ContextTryResult : int { Success = 0, CreateFailed = 1, MakeCurrentFailed = 2 };

static ContextTryResult TryCreateModernContext(HDC dc,HGLRC previous,PFNWGLCREATECONTEXTATTRIBSARBPROC_RE createAttribs,int major,int minor,int profileMask,HGLRC* outContext,DWORD* outError)
{
    const int attribs[]={WGL_CONTEXT_MAJOR_VERSION_ARB_,major,WGL_CONTEXT_MINOR_VERSION_ARB_,minor,WGL_CONTEXT_FLAGS_ARB_,0,WGL_CONTEXT_PROFILE_MASK_ARB_,profileMask,0};
    SetLastError(ERROR_SUCCESS);
    HGLRC candidate=createAttribs(dc,nullptr,attribs);
    if(!candidate){if(outError)*outError=GetLastError();return ContextTryResult::CreateFailed;}
    wglMakeCurrent(nullptr,nullptr);
    if(!wglMakeCurrent(dc,candidate)){if(outError)*outError=GetLastError();wglDeleteContext(candidate);if(previous)wglMakeCurrent(dc,previous);return ContextTryResult::MakeCurrentFailed;}
    if(outContext)*outContext=candidate;if(outError)*outError=ERROR_SUCCESS;return ContextTryResult::Success;
}

bool CreateRequestedContext(HDC dc,HGLRC legacyContext,RendererMode requested,bool requestCore46,ContextAttempt& result)
{
    result={};result.requested=requested;result.selected=RendererMode::LegacyGL2;result.requestedCore46=requestCore46;result.context=legacyContext;
    if(!dc||!legacyContext)return false;
    if(requested==RendererMode::LegacyGL2)return true;
    auto createAttribs=reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC_RE>(wglGetProcAddress("wglCreateContextAttribsARB"));
    result.hasCreateContextAttribs=createAttribs!=nullptr;
    if(!createAttribs){result.selectionChanged=true;return true;}
    HGLRC modern{};DWORD error{};
    if(requested==RendererMode::OpenGL46){result.tried46=true;const int profile=requestCore46?WGL_CONTEXT_CORE_PROFILE_BIT_ARB_:WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB_;if(TryCreateModernContext(dc,legacyContext,createAttribs,4,6,profile,&modern,&error)==ContextTryResult::Success){result.context=modern;result.selected=RendererMode::OpenGL46;return true;}result.lastError=error;}
    result.tried33=true;
    if(TryCreateModernContext(dc,legacyContext,createAttribs,3,3,WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB_,&modern,&error)==ContextTryResult::Success){result.context=modern;result.selected=RendererMode::OpenGL33Compatibility;result.selectionChanged=requested!=result.selected;return true;}
    result.lastError=error;result.context=legacyContext;result.selected=RendererMode::LegacyGL2;result.selectionChanged=requested!=result.selected;wglMakeCurrent(dc,legacyContext);return true;
}

} // namespace reverse_gl46
#endif
