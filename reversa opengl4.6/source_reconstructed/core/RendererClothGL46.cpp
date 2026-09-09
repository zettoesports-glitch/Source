#include "../include/RendererClothGL46.h"
namespace reverse_gl46 {
void RendererClothGL46Reference::Upload(const ClothVertex* v,std::size_t n,const std::uint32_t* i,std::size_t m){
    vertices_.clear();
    indices_.clear();
    if (v && n) vertices_.assign(v, v+n);
    if (i && m) indices_.assign(i, i+m);
}
}
