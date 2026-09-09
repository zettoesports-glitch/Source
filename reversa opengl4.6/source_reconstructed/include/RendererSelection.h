#pragma once
#include "RendererCommon.h"
namespace reverse_gl46 {
RendererMode ReadRendererModeFromRegistry();
const char* RendererModeName(RendererMode mode);
}
