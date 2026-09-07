#include "stdafx.h"
#include "BMDModernMesh.h"
#include "../../ZzzBMD.h"

#include <algorithm>

namespace
{
    struct CornerKey
    {
        short Vertex;
        short TexCoord;
        short Normal;
    };

    static bool SameCorner(const CornerKey& a, short v, short t, short n)
    {
        return a.Vertex == v && a.TexCoord == t && a.Normal == n;
    }

    static BMDModernVertex MakeVertex(const _Mesh_t& mesh, short v, short t, short n)
    {
        BMDModernVertex out = {};

        if (v >= 0 && v < mesh.NumVertices && mesh.Vertices != NULL)
        {
            const Vertex_t& sourceVertex = mesh.Vertices[v];
            out.Position[0] = sourceVertex.Position[0];
            out.Position[1] = sourceVertex.Position[1];
            out.Position[2] = sourceVertex.Position[2];

            const short positionBone = sourceVertex.Node;
            const std::uint32_t safePositionBone =
                positionBone >= 0 ? static_cast<std::uint32_t>(positionBone) : 0u;

            out.Bones[0] = safePositionBone;
            out.Bones[1] = safePositionBone;
        }

        if (t >= 0 && t < mesh.NumTexCoords && mesh.TexCoords != NULL)
        {
            out.UV[0] = mesh.TexCoords[t].TexCoordU;
            out.UV[1] = mesh.TexCoords[t].TexCoordV;
        }

        if (n >= 0 && n < mesh.NumNormals && mesh.Normals != NULL)
        {
            const Normal_t& sourceNormal = mesh.Normals[n];
            out.Normal[0] = sourceNormal.Normal[0];
            out.Normal[1] = sourceNormal.Normal[1];
            out.Normal[2] = sourceNormal.Normal[2];

            if (sourceNormal.Node >= 0)
                out.Bones[1] = static_cast<std::uint32_t>(sourceNormal.Node);
        }

        return out;
    }
}

bool BuildBMDModernMesh(const _Mesh_t& mesh, BMDModernMeshData& outMesh)
{
    outMesh.Clear();

    if (mesh.NumTriangles <= 0 || mesh.Triangles == NULL ||
        mesh.NumVertices <= 0 || mesh.Vertices == NULL)
        return false;

    std::vector<CornerKey> corners;
    corners.reserve(static_cast<size_t>(mesh.NumTriangles) * 3u);
    outMesh.Vertices.reserve(static_cast<size_t>(mesh.NumTriangles) * 3u);
    outMesh.Indices.reserve(static_cast<size_t>(mesh.NumTriangles) * 3u);

    for (int triangleIndex = 0; triangleIndex < mesh.NumTriangles; ++triangleIndex)
    {
        const Triangle_t& triangle = mesh.Triangles[triangleIndex];
        const int polygon = (std::min)(3, static_cast<int>(triangle.Polygon));

        for (int corner = 0; corner < polygon; ++corner)
        {
            const short v = triangle.VertexIndex[corner];
            const short t = triangle.TexCoordIndex[corner];
            const short n = triangle.NormalIndex[corner];

            if (v < 0 || v >= mesh.NumVertices)
                continue;

            std::uint32_t index = 0;
            bool found = false;
            for (size_t existing = 0; existing < corners.size(); ++existing)
            {
                if (SameCorner(corners[existing], v, t, n))
                {
                    index = static_cast<std::uint32_t>(existing);
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                index = static_cast<std::uint32_t>(outMesh.Vertices.size());
                CornerKey key = { v, t, n };
                corners.push_back(key);
                outMesh.Vertices.push_back(MakeVertex(mesh, v, t, n));
            }

            outMesh.Indices.push_back(index);
        }
    }

    return !outMesh.Empty();
}
