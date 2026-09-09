#pragma once
#include "RendererCommon.h"
namespace reverse_gl46 { class RendererSkyGL46Reference { public: void BeginFrame(const FrameDataStd140& f){frame_=f;} private: FrameDataStd140 frame_{}; }; }
