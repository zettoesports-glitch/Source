#include "../include/RendererModel.h"
#include <algorithm>
namespace reverse_gl46 {
std::uint32_t DecodeBoneIndex(std::uint32_t encodedBone) { return encodedBone / 3u; }
void RendererModelReference::BeginFrame(const FrameDataStd140& frame){ frame_=frame; queue_.clear(); }
void RendererModelReference::Queue(ModelDrawCommand command){ queue_.push_back(command); }
void RendererModelReference::ClearQueue(){ queue_.clear(); }
void RendererModelReference::SortOpaqueConservatively(){
    // RECONSTRUCTED from the literal stage name/telemetry:
    // G07ModelOpaqueSort algorithm=contiguous_opaque_same_pose_program_geometry.
    // Transparent commands are barriers. We only reorder a contiguous compatible run.
    std::size_t begin=0;
    while(begin<queue_.size()){
        if(queue_[begin].transparent){++begin;continue;}
        const auto* pose=queue_[begin].pose; const auto program=queue_[begin].program; const auto vao=queue_[begin].vertexArray;
        std::size_t end=begin+1;
        while(end<queue_.size() && !queue_[end].transparent && queue_[end].pose==pose && queue_[end].program==program && queue_[end].vertexArray==vao) ++end;
        std::stable_sort(queue_.begin()+begin,queue_.begin()+end,[](const ModelDrawCommand&a,const ModelDrawCommand&b){return a.texture<b.texture;});
        begin=end;
    }
}
}
