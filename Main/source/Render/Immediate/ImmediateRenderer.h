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
        LineLoop,
        Triangles,
        TriangleStrip,
        TriangleFan,
        Quads,
        QuadStrip,
        Polygon
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

    struct ImmediateDrawBatch
    {
        ImmediatePrimitive primitive;
        size_t firstVertex;
        size_t vertexCount;
    };

    // CPU staging buffer used as the migration bridge from glBegin/glEnd.
    // The renderer owns a fixed-capacity ring and never requires a legacy
    // call site to know which GPU backend will eventually consume it.
    class ImmediateRenderer
    {
    public:
        static const size_t DefaultCapacity = 1024 * 1024;

        explicit ImmediateRenderer(size_t capacity = DefaultCapacity)
            : m_capacity(capacity ? capacity : DefaultCapacity),
              m_writeOffset(0), m_frameOpen(false), m_inPrimitive(false) {}

        void BeginFrame()
        {
            m_vertices.clear();
            m_batches.clear();
            m_writeOffset = 0;
            m_frameOpen = true;
            m_inPrimitive = false;
        }

        void EndFrame() { m_frameOpen = false; m_inPrimitive = false; }

        void Begin(ImmediatePrimitive primitive)
        {
            if (!m_frameOpen)
                BeginFrame();
            if (m_inPrimitive)
                End();

            ImmediateBatch batch;
            batch.primitive = primitive;
            batch.firstVertex = m_vertices.size();
            batch.vertexCount = 0;
            m_batches.push_back(batch);
            m_inPrimitive = true;
        }

        bool Vertex(float x, float y, float z,
                    float r, float g, float b, float a,
                    float u, float v,
                    float nx = 0.0f, float ny = 0.0f, float nz = 1.0f)
        {
            if (!m_inPrimitive || m_batches.empty())
                return false;

            if (m_writeOffset + sizeof(ImmediateVertex) > m_capacity)
            {
                // Start a fresh ring segment. The caller can submit the
                // already accumulated batches before continuing.
                m_writeOffset = 0;
            }

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
            return true;
        }

        void End() { m_inPrimitive = false; }

        // Converts legacy primitives to core-friendly GL primitive classes.
        // Quads, quad strips, polygons and fans become triangles; line loops
        // become ordinary line lists. Returned vertices are contiguous.
        size_t BuildDrawBatches(std::vector<ImmediateVertex>& output,
                                std::vector<ImmediateDrawBatch>& draws) const
        {
            output.clear();
            draws.clear();

            for (size_t b = 0; b < m_batches.size(); ++b)
            {
                const ImmediateBatch& batch = m_batches[b];
                const size_t end = batch.firstVertex + batch.vertexCount;
                if (end > m_vertices.size())
                    continue;

                const size_t outputStart = output.size();
                ConvertBatch(batch, output);
                const size_t produced = output.size() - outputStart;
                if (produced == 0)
                    continue;

                ImmediateDrawBatch draw;
                draw.primitive = CorePrimitive(batch.primitive);
                draw.firstVertex = outputStart;
                draw.vertexCount = produced;
                draws.push_back(draw);
            }
            return output.size();
        }

        void ResetRing() { m_writeOffset = 0; }
        const std::vector<ImmediateVertex>& GetVertices() const { return m_vertices; }
        const std::vector<ImmediateBatch>& GetBatches() const { return m_batches; }
        size_t GetWriteOffset() const { return m_writeOffset; }
        size_t GetCapacity() const { return m_capacity; }
        bool IsFrameOpen() const { return m_frameOpen; }

    private:
        static ImmediatePrimitive CorePrimitive(ImmediatePrimitive primitive)
        {
            switch (primitive)
            {
            case ImmediatePrimitive::Quads:
            case ImmediatePrimitive::QuadStrip:
            case ImmediatePrimitive::Polygon:
            case ImmediatePrimitive::TriangleFan:
            case ImmediatePrimitive::TriangleStrip:
                return ImmediatePrimitive::Triangles;
            case ImmediatePrimitive::LineLoop:
                return ImmediatePrimitive::Lines;
            default:
                return primitive;
            }
        }

        void AddTriangle(std::vector<ImmediateVertex>& output,
                         const ImmediateVertex& a,
                         const ImmediateVertex& b,
                         const ImmediateVertex& c) const
        {
            output.push_back(a);
            output.push_back(b);
            output.push_back(c);
        }

        void AddLine(std::vector<ImmediateVertex>& output,
                     const ImmediateVertex& a,
                     const ImmediateVertex& b) const
        {
            output.push_back(a);
            output.push_back(b);
        }

        void ConvertBatch(const ImmediateBatch& batch,
                          std::vector<ImmediateVertex>& output) const
        {
            const size_t first = batch.firstVertex;
            const size_t count = batch.vertexCount;
            if (count == 0)
                return;

            switch (batch.primitive)
            {
            case ImmediatePrimitive::Quads:
                for (size_t i = 0; i + 3 < count; i += 4)
                {
                    AddTriangle(output, m_vertices[first + i], m_vertices[first + i + 1], m_vertices[first + i + 2]);
                    AddTriangle(output, m_vertices[first + i], m_vertices[first + i + 2], m_vertices[first + i + 3]);
                }
                break;

            case ImmediatePrimitive::QuadStrip:
                for (size_t i = 0; i + 3 < count; i += 2)
                {
                    AddTriangle(output, m_vertices[first + i], m_vertices[first + i + 1], m_vertices[first + i + 3]);
                    AddTriangle(output, m_vertices[first + i], m_vertices[first + i + 3], m_vertices[first + i + 2]);
                }
                break;

            case ImmediatePrimitive::Polygon:
            case ImmediatePrimitive::TriangleFan:
                for (size_t i = 1; i + 1 < count; ++i)
                    AddTriangle(output, m_vertices[first], m_vertices[first + i], m_vertices[first + i + 1]);
                break;

            case ImmediatePrimitive::TriangleStrip:
                for (size_t i = 0; i + 2 < count; ++i)
                {
                    if ((i & 1) == 0)
                        AddTriangle(output, m_vertices[first + i], m_vertices[first + i + 1], m_vertices[first + i + 2]);
                    else
                        AddTriangle(output, m_vertices[first + i + 1], m_vertices[first + i], m_vertices[first + i + 2]);
                }
                break;

            case ImmediatePrimitive::LineStrip:
                for (size_t i = 0; i + 1 < count; ++i)
                    AddLine(output, m_vertices[first + i], m_vertices[first + i + 1]);
                break;

            case ImmediatePrimitive::LineLoop:
                for (size_t i = 0; i + 1 < count; ++i)
                    AddLine(output, m_vertices[first + i], m_vertices[first + i + 1]);
                if (count > 2)
                    AddLine(output, m_vertices[first + count - 1], m_vertices[first]);
                break;

            default:
                for (size_t i = 0; i < count; ++i)
                    output.push_back(m_vertices[first + i]);
                break;
            }
        }

        size_t m_capacity;
        size_t m_writeOffset;
        bool m_frameOpen;
        bool m_inPrimitive;
        std::vector<ImmediateVertex> m_vertices;
        std::vector<ImmediateBatch> m_batches;
    };
}
