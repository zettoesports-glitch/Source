#include "stdafx.h"
#include "BMDModernMesh.h"
#include "../../ZzzBMD.h"

#include <algorithm>
#include <fstream>
#include <limits>

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

            // Preserve the real legacy VAO contract. VertexBMD stores one bone
            // (Vertex_t::Node * 3), and Model.vs uses that same bone for both
            // position and normal transforms. Some BMDs carry Normal_t::Node
            // values that differ from the vertex node; using those here caused
            // lighting/appearance divergence on NPCs even though player gear
            // happened to look correct.
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
        }

        return out;
    }

    static void LogMeshScan(int sourceVertices,
                            int sourceTriangles,
                            std::uint32_t referencedCorners,
                            std::uint32_t outputVertices,
                            std::uint32_t outputIndices,
                            int minBone,
                            int maxBone,
                            std::uint32_t negativeBoneRefs,
                            std::uint32_t invalidVertexRefs,
                            std::uint32_t invalidTexcoordRefs,
                            std::uint32_t invalidNormalRefs)
    {
        char message[512] = { 0 };
        sprintf_s(message,
                  "[ModernBMD] mesh bone scan: srcVertices=%d triangles=%d corners=%u outVertices=%u indices=%u minBone=%d maxBone=%d negativeBoneRefs=%u invalidVertexRefs=%u invalidTexcoordRefs=%u invalidNormalRefs=%u\n",
                  sourceVertices,
                  sourceTriangles,
                  referencedCorners,
                  outputVertices,
                  outputIndices,
                  minBone,
                  maxBone,
                  negativeBoneRefs,
                  invalidVertexRefs,
                  invalidTexcoordRefs,
                  invalidNormalRefs);

        OutputDebugStringA(message);
        std::ofstream logFile("Data\\ModernBMD.log", std::ios::out | std::ios::app);
        if (logFile.is_open())
            logFile << message;
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

    int minBone = (std::numeric_limits<int>::max)();
    int maxBone = (std::numeric_limits<int>::min)();
    std::uint32_t referencedCorners = 0;
    std::uint32_t negativeBoneRefs = 0;
    std::uint32_t invalidVertexRefs = 0;
    std::uint32_t invalidTexcoordRefs = 0;
    std::uint32_t invalidNormalRefs = 0;

    for (int triangleIndex = 0; triangleIndex < mesh.NumTriangles; ++triangleIndex)
    {
        const Triangle_t& triangle = mesh.Triangles[triangleIndex];
        const int polygon = (std::min)(3, static_cast<int>(triangle.Polygon));

        for (int corner = 0; corner < polygon; ++corner)
        {
            ++referencedCorners;

            const short v = triangle.VertexIndex[corner];
            const short t = triangle.TexCoordIndex[corner];
            const short n = triangle.NormalIndex[corner];

            if (v < 0 || v >= mesh.NumVertices)
            {
                ++invalidVertexRefs;
                continue;
            }

            const int bone = static_cast<int>(mesh.Vertices[v].Node);
            if (bone < 0)
            {
                ++negativeBoneRefs;
            }
            else
            {
                minBone = (std::min)(minBone, bone);
                maxBone = (std::max)(maxBone, bone);
            }

            if (t < 0 || t >= mesh.NumTexCoords || mesh.TexCoords == NULL)
                ++invalidTexcoordRefs;
            if (n < 0 || n >= mesh.NumNormals || mesh.Normals == NULL)
                ++invalidNormalRefs;

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

    if (minBone == (std::numeric_limits<int>::max)())
        minBone = -1;
    if (maxBone == (std::numeric_limits<int>::min)())
        maxBone = -1;

    LogMeshScan(mesh.NumVertices,
                mesh.NumTriangles,
                referencedCorners,
                static_cast<std::uint32_t>(outMesh.Vertices.size()),
                static_cast<std::uint32_t>(outMesh.Indices.size()),
                minBone,
                maxBone,
                negativeBoneRefs,
                invalidVertexRefs,
                invalidTexcoordRefs,
                invalidNormalRefs);

    // Invalid source vertex references, negative nodes, or a node outside the
    // globally supported skeleton range are unsafe for the texture-backed
    // atlas. Reject the mesh so a malformed/foreign node can never read into
    // the next pose allocation in BonesTexture.
    if (invalidVertexRefs != 0 || negativeBoneRefs != 0 || maxBone >= MAX_BONES)
    {
        outMesh.Clear();
        return false;
    }

    return !outMesh.Empty();
}
