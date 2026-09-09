#include "../include/RendererSelection.h"
#include "../include/RenderDevice3D.h"
#ifdef _WIN32
namespace reverse_gl46 {
// RECONSTRUCTED bootstrap: exact surrounding WinMain call graph is not yet recovered.
bool BootstrapRenderer(HDC dc,HGLRC legacy,bool requestCore46,ContextAttempt& attempt){
    const RendererMode requested=ReadRendererModeFromRegistry();
    return CreateRequestedContext(dc,legacy,requested,requestCore46,attempt);
}
}
#endif
