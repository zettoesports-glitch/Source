#include "../include/RendererSelection.h"
#ifdef _WIN32
#include <windows.h>
namespace reverse_gl46 {
RendererMode ReadRendererModeFromRegistry() {
    constexpr const char* kKey="SOFTWARE\\Webzen\\Mu2\\Config";
    constexpr const char* kValue="OpenGlUseShader";
    DWORD value=1; HKEY key{};
    if (RegOpenKeyExA(HKEY_CURRENT_USER,kKey,0,KEY_READ,&key)==ERROR_SUCCESS) {
        DWORD type=REG_DWORD,size=sizeof(value);
        if (RegQueryValueExA(key,kValue,nullptr,&type,reinterpret_cast<BYTE*>(&value),&size)!=ERROR_SUCCESS || type!=REG_DWORD) value=1;
        RegCloseKey(key);
    }
    if (value>2) value=1;
    return static_cast<RendererMode>(value);
}
const char* RendererModeName(RendererMode mode) {
    switch(mode){case RendererMode::LegacyGL2:return "OpenGL 2 (legacy)";case RendererMode::OpenGL33Compatibility:return "OpenGL 3.3 Compatibility";case RendererMode::OpenGL46:return "OpenGL 4.6";default:return "OpenGL 3.3 Compatibility";}
}
}
#endif
