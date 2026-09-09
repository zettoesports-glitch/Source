#pragma once
#include "RendererCommon.h"
#include <cstdint>
#include <cstddef>
#include <vector>
namespace reverse_gl46 {
struct ClothVertex { Vec3 position{}; Vec3 normal{}; Vec2 uv{}; Vec4 color{1,1,1,1}; };
class RendererClothGL46Reference { public: void Upload(const ClothVertex* v,std::size_t n,const std::uint32_t* i,std::size_t m); std::size_t VertexCount()const{return vertices_.size();} private: std::vector<ClothVertex> vertices_; std::vector<std::uint32_t> indices_; };
}
