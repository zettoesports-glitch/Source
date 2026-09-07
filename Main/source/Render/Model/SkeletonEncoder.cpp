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
        // Matches ZzzMathLib::VectorTransform exactly:
        // each output component is the dot product with one affine row.
        return { v.x*m[0] + v.y*m[1] + v.z*m[2] + m[3],
                 v.x*m[4] + v.y*m[5] + v.z*m[6] + m[7],
                 v.x*m[8] + v.y*m[9] + v.z*m[10] + m[11] };
    }

    static Vec3 RotateCpu(const float* m, Vec3 v)
    {
        // Matches ZzzMathLib::VectorRotate exactly.
        return { v.x*m[0] + v.y*m[1] + v.z*m[2],
                 v.x*m[4] + v.y*m[5] + v.z*m[6],
                 v.x*m[8] + v.y*m[9] + v.z*m[10] };
    }

    static Vec3 ShaderMatrixPoint(const float* c, Vec3 v)
    {
        // c contains the four GLSL mat4 columns / HLSL transposed rows emitted
        // by EncodeMatrix4x4. This reproduces the shader result, not CPU memory
        // indexing, so a transpose bug cannot be hidden by the validator.
        return { c[0]*v.x + c[4]*v.y + c[8]*v.z + c[12],
                 c[1]*v.x + c[5]*v.y + c[9]*v.z + c[13],
                 c[2]*v.x + c[6]*v.y + c[10]*v.z + c[14] };
    }

    static Vec3 ShaderMatrixNormal(const float* c, Vec3 v)
    {
        return { c[0]*v.x + c[4]*v.y + c[8]*v.z,
                 c[1]*v.x + c[5]*v.y + c[9]*v.z,
                 c[2]*v.x + c[6]*v.y + c[10]*v.z };
    }

    static float Len(Vec3 v) { return std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z); }
    static float Dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

    static float Determinant3x3(const float* m)
    {
        return m[0] * (m[5] * m[10] - m[6] * m[9])
             - m[1] * (m[4] * m[10] - m[6] * m[8])
             + m[2] * (m[4] * m[9] - m[5] * m[8]);
    }

    static bool NormalizeQuaternion(Quat& q, float epsilon)
    {
        const float length = std::sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
        if (!std::isfinite(length) || length <= epsilon)
            return false;

        const float inv = 1.0f / length;
        q.x *= inv;
        q.y *= inv;
        q.z *= inv;
        q.w *= inv;
        return std::isfinite(q.x) && std::isfinite(q.y) &&
               std::isfinite(q.z) && std::isfinite(q.w);
    }

    static Quat MatrixToQuaternion(const float* m)
    {
        // m is the MU CPU row-major rotation matrix. The signs/order below are
        // the inverse of Main/source/Math/ZzzMathLib.cpp::QuaternionMatrix and
        // therefore match SkeletonHelpers.inc (x, y, z, w) without conjugation.
        const float trace = m[0] + m[5] + m[10];
        Quat q;
        if (trace > 0.0f)
        {
            const float s = std::sqrt(trace + 1.0f) * 2.0f;
            q.w = 0.25f * s;
            q.x = (m[9] - m[6]) / s;
            q.y = (m[2] - m[8]) / s;
            q.z = (m[4] - m[1]) / s;
        }
        else if (m[0] > m[5] && m[0] > m[10])
        {
            const float s = std::sqrt(1.0f + m[0] - m[5] - m[10]) * 2.0f;
            q.w = (m[9] - m[6]) / s;
            q.x = 0.25f * s;
            q.y = (m[1] + m[4]) / s;
            q.z = (m[2] + m[8]) / s;
        }
        else if (m[5] > m[10])
        {
            const float s = std::sqrt(1.0f + m[5] - m[0] - m[10]) * 2.0f;
            q.w = (m[2] - m[8]) / s;
            q.x = (m[1] + m[4]) / s;
            q.y = 0.25f * s;
            q.z = (m[6] + m[9]) / s;
        }
        else
        {
            const float s = std::sqrt(1.0f + m[10] - m[0] - m[5]) * 2.0f;
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

        // Exact CPU-side equivalent of SkeletonHelpers.inc::RequestBone after
        // its transposed affine matrix is consumed by mul(position, matrix).
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
    if (!matrices || count == 0)
    {
        out.clear();
        return;
    }

    out.resize(static_cast<size_t>(count) * 16u);
    for (std::uint32_t i=0; i<count; ++i)
    {
        const float* m = matrices + static_cast<size_t>(i)*12u;
        float* d = out.data() + static_cast<size_t>(i)*16u;

        // MU CPU affine memory is three ROWS:
        // [m0 m1 m2 m3], [m4 m5 m6 m7], [m8 m9 m10 m11].
        // The HLSL shader uses row-vector mul(position, matrix), so RequestBone
        // expects the transposed affine matrix. In generated GLSL these same
        // four float4 values become mat4 columns and matrix * vector is used.
        d[0]=m[0];  d[1]=m[4];  d[2]=m[8];   d[3]=0.0f;
        d[4]=m[1];  d[5]=m[5];  d[6]=m[9];   d[7]=0.0f;
        d[8]=m[2];  d[9]=m[6];  d[10]=m[10]; d[11]=0.0f;
        d[12]=m[3]; d[13]=m[7]; d[14]=m[11]; d[15]=1.0f;
    }
}

bool SkeletonEncoder::EncodeQuaternionPositionScale(const float* matrices,
                                                     std::uint32_t count,
                                                     std::vector<float>& out,
                                                     float epsilon)
{
    if (!matrices || count == 0 || epsilon <= 0.0f)
    {
        out.clear();
        return false;
    }

    out.resize(static_cast<size_t>(count) * 8u);
    for (std::uint32_t i=0; i<count; ++i)
    {
        const float* src = matrices + static_cast<size_t>(i)*12u;

        // Uniform scale is measured from the three MU rotation ROWS because
        // VectorTransform/VectorRotate dot the input vector against each row.
        Vec3 row0{src[0],src[1],src[2]};
        Vec3 row1{src[4],src[5],src[6]};
        Vec3 row2{src[8],src[9],src[10]};
        const float sx=Len(row0), sy=Len(row1), sz=Len(row2);
        const float absScale=(sx+sy+sz)/3.0f;
        const float scaleTolerance = epsilon * (std::max)(1.0f, absScale);

        if (!std::isfinite(absScale) || absScale <= epsilon ||
            std::fabs(sx-absScale)>scaleTolerance ||
            std::fabs(sy-absScale)>scaleTolerance ||
            std::fabs(sz-absScale)>scaleTolerance)
        {
            out.clear();
            return false;
        }

        // A negative determinant is representable by a negative uniform scale
        // plus a proper rotation. This also makes mirrored-but-orthogonal data
        // deterministic rather than silently producing an invalid quaternion.
        const float determinant = Determinant3x3(src);
        if (!std::isfinite(determinant) || std::fabs(determinant) <= epsilon)
        {
            out.clear();
            return false;
        }
        const float scale = determinant < 0.0f ? -absScale : absScale;

        float r[12] = {};
        r[0]=src[0]/scale;   r[1]=src[1]/scale;   r[2]=src[2]/scale;
        r[4]=src[4]/scale;   r[5]=src[5]/scale;   r[6]=src[6]/scale;
        r[8]=src[8]/scale;   r[9]=src[9]/scale;   r[10]=src[10]/scale;

        Vec3 rr0{r[0],r[1],r[2]}, rr1{r[4],r[5],r[6]}, rr2{r[8],r[9],r[10]};
        if (std::fabs(Dot(rr0,rr1))>epsilon ||
            std::fabs(Dot(rr0,rr2))>epsilon ||
            std::fabs(Dot(rr1,rr2))>epsilon ||
            std::fabs(Len(rr0)-1.0f)>epsilon ||
            std::fabs(Len(rr1)-1.0f)>epsilon ||
            std::fabs(Len(rr2)-1.0f)>epsilon ||
            std::fabs(Determinant3x3(r)-1.0f)>(epsilon*4.0f))
        {
            out.clear();
            return false;
        }

        Quat q=MatrixToQuaternion(r);
        if (!NormalizeQuaternion(q, epsilon))
        {
            out.clear();
            return false;
        }

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
    if (!m || !p || !n || samples==0 || epsilon<=0.0f) return r;
    std::vector<float> encoded;
    EncodeMatrix4x4(m,1,encoded);
    if (encoded.size()!=16u) return r;

    for (std::uint32_t i=0;i<samples;++i)
    {
        Vec3 v{p[i*3],p[i*3+1],p[i*3+2]};
        Vec3 nn{n[i*3],n[i*3+1],n[i*3+2]};
        Vec3 cpu=TransformPointCpu(m,v);
        Vec3 gpu=ShaderMatrixPoint(encoded.data(),v);
        Vec3 cn=RotateCpu(m,nn), gn=ShaderMatrixNormal(encoded.data(),nn);
        r.MaxPositionError=(std::max)(r.MaxPositionError,Len({cpu.x-gpu.x,cpu.y-gpu.y,cpu.z-gpu.z}));
        r.MaxNormalError=(std::max)(r.MaxNormalError,Len({cn.x-gn.x,cn.y-gn.y,cn.z-gn.z}));
    }
    r.Success=r.MaxPositionError<=epsilon && r.MaxNormalError<=epsilon;
    return r;
}

SkeletonEncoder::ValidationResult SkeletonEncoder::ValidateQuaternionEncoding(
    const float* m, const float* p, const float* n, std::uint32_t samples, float epsilon)
{
    ValidationResult r;
    if (!m || !p || !n || samples==0 || epsilon<=0.0f) return r;
    std::vector<float> e;
    if (!EncodeQuaternionPositionScale(m,1,e,epsilon) || e.size()!=8u) return r;
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
        r.MaxPositionError=(std::max)(r.MaxPositionError,Len({cpu.x-gpu.x,cpu.y-gpu.y,cpu.z-gpu.z}));
        r.MaxNormalError=(std::max)(r.MaxNormalError,Len({cn.x-gn.x,cn.y-gn.y,cn.z-gn.z}));
    }
    r.Success=r.MaxPositionError<=epsilon && r.MaxNormalError<=epsilon;
    return r;
}