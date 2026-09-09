#pragma once
#include "RendererCommon.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
namespace reverse_gl46 {
struct Draw2DCommand { std::uint32_t texture{}; float x{},y{},w{},h{}; Vec4 color{1,1,1,1}; bool ordered{}; };
class Renderer2DGL46Reference {
public:
    void BeginFrame(const FrameDataStd140& frame);
    void Submit(const Draw2DCommand& cmd);
    void SubmitMonsterHpBar(const Draw2DCommand& cmd);
    void CacheText(std::uint64_t key, std::uint32_t texture);
    void Clear();
private:
    FrameDataStd140 frame_{};
    std::vector<Draw2DCommand> ordered_{};
    std::vector<Draw2DCommand> hpBars_{};
    std::unordered_map<std::uint64_t,std::uint32_t> textCache_{};
};
}
