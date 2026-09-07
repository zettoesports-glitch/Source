#pragma once

#include <vector>

#include "../../CoreGLCompat.h"
#include "ImmediateRenderer.h"

namespace Render
{
    // Controlled migration adapter for legacy immediate-mode consumers.
    // ImmediateRenderer owns primitive conversion/staging; CoreGLCompat remains
    // the compatibility submission bridge until the consumer moves to the RHI.
    inline void DrawQuadThroughImmediate(const float positions[4][3],
                                         const float texCoords[4][2],
                                         const float color[4])
    {
        ImmediateRenderer renderer(4 * sizeof(ImmediateVertex) * 2);
        renderer.BeginFrame();
        renderer.Begin(ImmediatePrimitive::Quads);

        for (int i = 0; i < 4; ++i)
        {
            renderer.Vertex(
                positions[i][0], positions[i][1], positions[i][2],
                color[0], color[1], color[2], color[3],
                texCoords[i][0], texCoords[i][1]);
        }

        renderer.End();

        std::vector<ImmediateVertex> converted;
        std::vector<ImmediateDrawBatch> draws;
        renderer.BuildDrawBatches(converted, draws);

        for (size_t drawIndex = 0; drawIndex < draws.size(); ++drawIndex)
        {
            const ImmediateDrawBatch& draw = draws[drawIndex];
            if (draw.primitive != ImmediatePrimitive::Triangles || draw.vertexCount == 0)
                continue;

            CoreGLCompat::Begin(GL_TRIANGLES);
            for (size_t i = 0; i < draw.vertexCount; ++i)
            {
                const ImmediateVertex& vertex = converted[draw.firstVertex + i];
                CoreGLCompat::Color4f(vertex.color[0], vertex.color[1], vertex.color[2], vertex.color[3]);
                CoreGLCompat::TexCoord2f(vertex.texCoord[0], vertex.texCoord[1]);
                CoreGLCompat::Vertex3f(vertex.position[0], vertex.position[1], vertex.position[2]);
            }
            CoreGLCompat::End();
        }

        renderer.EndFrame();
    }
}
