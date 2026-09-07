#include "stdafx.h"
#include "SkeletonEncoder.h"

#include <algorithm>
#include <cmath>

namespace
{
    struct Vec3 { float x, y, z; };
    struct Quat { float x, y, z, w; };

    static Vec3 TransformPointCpu(const float* m, Vec3 v)
    {
        return { v.x*m[0] + v.y*m[4] + v.z*m[8] + m[3],
                 v.x*m[1] + v.y*m[5] + v.z*m[9] + m[7],
                 v.x*m[2] + v.y*m[6] + v.z*m[10] + m[11] };
    }

    static Vec3 RotateCpu(const float* m, Vec3 v)
    {
        return { v.x*m[0] + v.y*m[4] + v.z*m[8],
                 v.x*m[1] + v.y*m[5] + v.z*m[9],
                 v.x*m[2] + v.y*m[6] + v.z*m[10] };
    }

    static float Len(Vec3 v) { return std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z); }
    static float Dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

    static Quat MatrixToQuaternion(const float* m)
    {
        // m is row-major rotation matrix. This is the standard conversion for
        // the same rotation represented by the shader's RequestBone formula.
        const float trace = m[0] + m[5] + m[10];
        Quat q;
        if (trace > 0.0f)
        {
            float s = std::sqrt(trace + 1.0f) * 2.0f;
            q.w = 0.25f * s;
            q.x = (m[9] - m[6]) / s;
            q.y = (m[2] - m[8]) / s;
            q.z = (m[4] - m[1]) / s;
        }
        else if (m[0] > m[5] && m[0] > m[10])
        {
            float s = std::sqrt(1.0f + m[0] - m[5] - m[10]) * 2.0f;
            q.w = (m[9] - m[6]) / s;
            q.x = 0.25f * s;
            q.y = (m[1] + m[4]) / s;
            q.z = (m[2] + m[8]) / s;
        }
        else if (m[5] > m[10])
        {
            float s = std::sqrt(1.0f + m[5] - m[0] - m[10]) * 2.0f;
            q.w = (m[2] - m[8]) / s;
            q.x = (m[1] + m[4]) / s;
            q.y = 0.25f * s;
            q.z = (m[6] + m[9]) / s;
        }
        else
        {
            float s = std::sqrt(1.0f + m[10] - m[0] - m[5]) * 2.0f;
            q.w = (m[4] - m[1]) / s;
            q.x = (m[2] + m[8]) / s;
            q.y = (m[6] + m[9]) / s;
            q.z = 0.25f * s;
        }
        return q;
    }

    static Vec3 ShaderQuaternionPoint(Quat q, Vec3 t, float scale, Vec3 v)
    {
        const float xx=q.x*q.x, yy=q.y*q.y, zz=q.z*q.z;
        const float xy=q.x*q.y, xz=q.x*q.z, yz=q.y*q.z;
        const float wx=q.w*q.x, wy=q.w*q.y, wz=q.w*q.z;
        // Matches the actual GLSL RequestBone() column-vector matrix.
        return {
            ((1-2*(yy+zz))*v.x + (2*(xy-wz))*v.y + (2*(xz+wy))*v.z)*scale + t.x,
            ((2*(xy+wz))*v.x + (1-2*(xx+zz))*v.y + (2*(yz-wx))*v.z)*scale + t.y,
            ((2*(xz-wy))*v.x + (2*(yz+wx))*v.y + (1-2*(xx+yy))*v.z)*scale + t.z
        };
    }
}

void SkeletonEncoder::EncodeMatrix4x4(const float* matrices,
                                      std::uint32_t count,
                                      std::vector<float>& out)
{
    out.resize(static_cast<size_t>(count) * 16u);
    for (std::uint32_t i=0; i<count; ++i)
    {
        const float* m = matrices + static_cast<size_t>(i)*12u;
        float* d = out.data() + static_cast<size_t>(i)*16u;

        // GLSL mat4(vec4...) constructor receives columns.
        d[0]=m[0];  d[1]=m[1];  d[2]=m[2];  d[3]=0.0f;
        d[4]=m[4];  d[5]=m[5];  d[6]=m[6];  d[7]=0.0f;
        d[8]=m[8];  d[9]=m[9];  d[10]=m[10]; d[11]=0.0f;
        d[12]=m[3]; d[13]=m[7]; d[14]=m[11]; d[15]=1.0f;
    }
}

bool SkeletonEncoder::EncodeQuaternionPositionScale(const float* matrices,
                                                     std::uint32_t count,
                                                     std::vector<float>& out,
                                                     float epsilon)
{
    out.resize(static_cast<size_t>(count) * 8u);
    for (std::uint32_t i=0; i<count; ++i)
    {
        const float* src = matrices + static_cast<size_t>(i)*12u;
        Vec3 c0{src[0],src[1],src[2]};
        Vec3 c1{src[4],src[5],src[6]};
        Vec3 c2{src[8],src[9],src[10]};
        const float sx=Len(c0), sy=Len(c1), sz=Len(c2);
        const float scale=(sx+sy+sz)/3.0f;
        if (scale <= epsilon || std::fabs(sx-scale)>epsilon ||
            std::fabs(sy-scale)>epsilon || std::fabs(sz-scale)>epsilon)
            return false;

        float r[12] = {};
        for (int k=0;k<12;++k) r[k]=src[k];
        r[0]/=scale; r[1]/=scale; r[2]/=scale;
        r[4]/=scale; r[5]/=scale; r[6]/=scale;
        r[8]/=scale; r[9]/=scale; r[10]/=scale;

        Vec3 rc0{r[0],r[1],r[2]}, rc1{r[4],r[5],r[6]}, rc2{r[8],r[9],r[10]};
        if (std::fabs(Dot(rc0,rc1))>epsilon ||
            std::fabs(Dot(rc0,rc2))>epsilon ||
            std::fabs(Dot(rc1,rc2))>epsilon)
            return false;

        Quat q=MatrixToQuaternion(r);
        float* d=out.data()+static_cast<size_t>(i)*8u;
        d[0]=q.x; d[1]=q.y; d[2]=q.z; d[3]=q.w;
        d[4]=src[3]; d[5]=src[7]; d[6]=src[11]; d[7]=scale;
    }
    return true;
}

SkeletonEncoder::ValidationResult SkeletonEncoder::ValidateMatrixEncoding(
    const float* m, const float* p, const float* n, std::uint32_t samples, float epsilon)
{
    ValidationResult r;
    if (!m || !p || !n || samples==0) return r;
    std::vector<float> encoded; EncodeMatrix4x4(m,1,encoded);
    for (std::uint32_t i=0;i<samples;++i)
    {
        Vec3 v{p[i*3],p[i*3+1],p[i*3+2]};
        Vec3 nn{n[i*3],n[i*3+1],n[i*3+2]};
        Vec3 cpu=TransformPointCpu(m,v);
        Vec3 gpu=TransformPointCpu(encoded.data(),v); // encoded columns reconstruct same affine under GLSL convention
        Vec3 cn=RotateCpu(m,nn), gn=RotateCpu(encoded.data(),nn);
        r.MaxPositionError=std::max(r.MaxPositionError,Len({cpu.x-gpu.x,cpu.y-gpu.y,cpu.z-gpu.z}));
        r.MaxNormalError=std::max(r.MaxNormalError,Len({cn.x-gn.x,cn.y-gn.y,cn.z-gn.z}));
    }
    r.Success=r.MaxPositionError<=epsilon && r.MaxNormalError<=epsilon;
    return r;
}

SkeletonEncoder::ValidationResult SkeletonEncoder::ValidateQuaternionEncoding(
    const float* m, const float* p, const float* n, std::uint32_t samples, float epsilon)
{
    ValidationResult r;
    if (!m || !p || !n || samples==0) return r;
    std::vector<float> e;
    if (!EncodeQuaternionPositionScale(m,1,e,epsilon)) return r;
    Quat q{e[0],e[1],e[2],e[3]};
    Vec3 t{e[4],e[5],e[6]}; float s=e[7];
    for (std::uint32_t i=0;i<samples;++i)
    {
        Vec3 v{p[i*3],p[i*3+1],p[i*3+2]};
        Vec3 nn{n[i*3],n[i*3+1],n[i*3+2]};
        Vec3 cpu=TransformPointCpu(m,v);
        Vec3 gpu=ShaderQuaternionPoint(q,t,s,v);
        Vec3 cn=RotateCpu(m,nn);
        Vec3 gn=ShaderQuaternionPoint(q,{0,0,0},s,nn);
        r.MaxPositionError=std::max(r.MaxPositionError,Len({cpu.x-gpu.x,cpu.y-gpu.y,cpu.z-gpu.z}));
        r.MaxNormalError=std::max(r.MaxNormalError,Len({cn.x-gn.x,cpu.y*0+gn.y-cn.y,gn.z-cn.z}));
    }
    r.Success=r.MaxPositionError<=epsilon && r.MaxNormalError<=epsilon;
    return r;
}