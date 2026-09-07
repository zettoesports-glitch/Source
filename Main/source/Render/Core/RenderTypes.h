#pragma once

#include <cstdint>

namespace Render
{
    using BufferHandle = uint32_t;
    using TextureHandle = uint32_t;
    using ShaderHandle = uint32_t;
    using PipelineHandle = uint32_t;

    static const BufferHandle InvalidBuffer = 0;
    static const TextureHandle InvalidTexture = 0;
    static const ShaderHandle InvalidShader = 0;
    static const PipelineHandle InvalidPipeline = 0;

    enum class BufferUsage : uint8_t
    {
        Static,
        Dynamic,
        Stream
    };

    enum class BufferType : uint8_t
    {
        Vertex,
        Index,
        Uniform
    };

    enum class IndexType : uint8_t
    {
        UInt16,
        UInt32
    };

    struct BufferDesc
    {
        size_t size;
        BufferUsage usage;
        BufferType type;

        BufferDesc() : size(0), usage(BufferUsage::Static), type(BufferType::Vertex) {}
    };

    struct DrawDesc
    {
        uint32_t vertexCount;
        uint32_t firstVertex;

        DrawDesc() : vertexCount(0), firstVertex(0) {}
    };

    struct DrawIndexedDesc
    {
        uint32_t indexCount;
        uint32_t firstIndex;
        int32_t vertexOffset;
        IndexType indexType;

        DrawIndexedDesc()
            : indexCount(0), firstIndex(0), vertexOffset(0), indexType(IndexType::UInt32) {}
    };
}
