#include "../include/Renderer2DGL46.h"
namespace reverse_gl46 {
void Renderer2DGL46Reference::BeginFrame(const FrameDataStd140& frame){frame_=frame;ordered_.clear();hpBars_.clear();}
void Renderer2DGL46Reference::Submit(const Draw2DCommand& cmd){ordered_.push_back(cmd);}
void Renderer2DGL46Reference::SubmitMonsterHpBar(const Draw2DCommand& cmd){hpBars_.push_back(cmd);}
void Renderer2DGL46Reference::CacheText(std::uint64_t key,std::uint32_t texture){textCache_[key]=texture;}
void Renderer2DGL46Reference::Clear(){ordered_.clear();hpBars_.clear();textCache_.clear();}
}
