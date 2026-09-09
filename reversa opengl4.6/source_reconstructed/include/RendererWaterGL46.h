#pragma once
#include "RendererCommon.h"
namespace reverse_gl46 {
struct WaterMaterialState { float level{}, depthRange{}, turbidity{}, frost{}, lava{}, move{}, time{}; };
class RendererWaterGL46Reference { public: void SetState(const WaterMaterialState& s){state_=s;} const WaterMaterialState& State() const{return state_;} private: WaterMaterialState state_{}; };
}
