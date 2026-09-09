// RECONSTRUÍDO a partir de Main.exe; não é a source original.
// Evidência: rotina em torno de VA 0x0077E550.
#include <windows.h>

namespace reverse_gl46 {

enum class RendererMode : int { LegacyGL2=0, OpenGL33Compatibility=1, OpenGL46=2 };

RendererMode ReadRendererModeFromRegistry()
{
    constexpr const char* kKey = "SOFTWARE\\Webzen\\Mu2\\Config";
    constexpr const char* kValue = "OpenGlUseShader";

    DWORD value = 1; // comportamento observado: 3.3 é o default/fallback de valor inválido
    HKEY key{};
    if (RegOpenKeyExA(HKEY_CURRENT_USER, kKey, 0, KEY_READ, &key) == ERROR_SUCCESS) {
        DWORD type = REG_DWORD;
        DWORD size = sizeof(value);
        if (RegQueryValueExA(key, kValue, nullptr, &type,
                            reinterpret_cast<BYTE*>(&value), &size) != ERROR_SUCCESS) {
            value = 1;
        }
        RegCloseKey(key);
    }

    if (value > 2) value = 1;
    return static_cast<RendererMode>(value);
}

const char* RendererModeName(RendererMode mode)
{
    switch (mode) {
    case RendererMode::LegacyGL2: return "OpenGL 2 (legacy)";
    case RendererMode::OpenGL33Compatibility: return "OpenGL 3.3 Compatibility";
    case RendererMode::OpenGL46: return "OpenGL 4.6";
    default: return "OpenGL 3.3 Compatibility";
    }
}

} // namespace reverse_gl46
