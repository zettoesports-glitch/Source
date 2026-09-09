#pragma once
#include "RendererCommon.h"
namespace reverse_gl46 {
struct AtmosphereParams { Vec3 color{}; float opacity{}; float time{}; };
class RendererAtmosphereGL46Reference { public: void SetParams(const AtmosphereParams& p){params_=p;} const AtmosphereParams& Params()const{return params_;} private: AtmosphereParams params_{}; };
}
