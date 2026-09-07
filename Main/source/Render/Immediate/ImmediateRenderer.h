#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Render
{
    enum class ImmediatePrimitive : uint8_t
    {
        Points,
        Lines,
        LineStrip,
        Triangles,
        TriangleStrip,
        TriangleFan,
        Quads
    };

    struct ImmediateVertex
    {
        float position[3];
        float color[4];
        float texCoord[2];
        float normal[3];
    };

    struct ImmediateBatch
    {
        ImmediatePrimitive primitive;
        size_t firstVertex;
        size_t vertexCount;
    };

    // CPU-side staging plus a ring-buffer allocator. Backend submission is
    // intentionally separate so CoreGLCompat remains the safe fallback.
    class ImmediateRenderer
    {
    public:
        static const size_t DefaultCapacity = 1024 * 1024;

        explicit ImmediateRenderer(size_t capacity = DefaultCapacity)
            : m_capacity(capacity), m_writeOffset(0), m_frameOpen(false) {}

        void BeginFrame()
        {
            m_vertices.clear();
            m_batches.clear();
            m_writeOffset = 0;
            m_frameOpen = true;
        }

        void EndFrame() { m_frameOpen = false; }

        void Begin(ImmediatePrimitive primitive)
        {
            if (!m_frameOpen)
                BeginFrame();
            ImmediateBatch batch;
            batch.primitive = primitive;
            batch.firstVertex = m_vertices.size();
            batch.vertexCount = 0;
            m_batches.push_back(batch);
        }

        void Vertex(float x, float y, float z,
                    float r, float g, float b, float a,
                    float u, float v,
                    float nx = 0.0f, float ny = 0.0f, float nz = 1.0f)
        {
            if (m_batches.empty())
                return;

            ImmediateVertex vertex;
            vertex.position[0] = x;
            vertex.position[1] = y;
            vertex.position[2] = z;
            vertex.color[0] = r;
            vertex.color[1] = g;
            vertex.color[2] = b;
            vertex.color[3] = a;
            vertex.texCoord[0] = u;
            vertex.texCoord[1] = v;
            vertex.normal[0] = nx;
            vertex.normal[1] = ny;
            vertex.normal[2] = nz;
            m_vertices.push_back(vertex);
            ++m_batches.back().vertexCount;
            m_writeOffset += sizeof(ImmediateVertex);

            if (m_writeOffset >= m_capacity)
                m_writeOffset = 0;
        }

        void End()
        {
            // Batches remain immutable until Submit() so multiple legacy
            // primitives can share one dynamic upload region.
        }

        void ResetRing() { m_writeOffset = 0; }

        const std::vector<ImmediateVertex>& GetVertices() const { return m_vertices; }
        const std::vector<ImmediateBatch>& GetBatches() const { return m_batches; }
        size_t GetWriteOffset() const { return m_writeOffset; }
        size_t GetCapacity() const { return m_capacity; }
        bool IsFrameOpen() const { return m_frameOpen; }

    private:
        size_t m_capacity;
        size_t m_writeOffset;
        bool m_frameOpen;
        std::vector<ImmediateVertex> m_vertices;
        std::vector<ImmediateBatch> m_batches;
    };
}
