#pragma once
#include "RendererCommon.h"
#ifdef _WIN32
#include <windows.h>
namespace reverse_gl46 {
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
bool CreateRequestedContext(HDC dc, HGLRC legacyContext, RendererMode requested,
                            bool requestCore46, ContextAttempt& result);
}
#endif
