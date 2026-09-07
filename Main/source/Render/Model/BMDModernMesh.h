#pragma once

#include <cstdint>
#include <vector>

struct _Mesh_t;

// Backend-neutral vertex contract shared by the generated OpenGL shaders and
// the Vulkan/HLSL source in common/VertexDefinitions.inc.
//
// location 0: Position
// location 1: UV
// location 2: Normal
// location 3: Bones[x = vertex node, y = normal node]
struct BMDModernVertex
{
    float Position[3];
    float UV[2];
    float Normal[3];
    std::uint32_t Bones[2];
};

struct BMDModernMeshData
{
    std::vector<BMDModernVertex> Vertices;
    std::vector<std::uint32_t> Indices;

    void Clear()
    {
        Vertices.clear();
        Indices.clear();
    }

    bool Empty() const
    {
        return Vertices.empty() || Indices.empty();
    }
};

// Converts the legacy BMD mesh into the immutable modern GPU vertex contract.
// This does not change or replace the legacy VAOMesh path.
bool BuildBMDModernMesh(const _Mesh_t& mesh, BMDModernMeshData& outMesh);
