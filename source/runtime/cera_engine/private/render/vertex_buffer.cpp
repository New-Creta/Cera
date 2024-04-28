#include "render/vertex_buffer.h"

namespace cera
{
    vertex_buffer::vertex_buffer(device& device, size_t numVertices, size_t vertexStride)
        : buffer(device, CD3DX12_RESOURCE_DESC::Buffer(numVertices* vertexStride))
        , m_num_vertices(numVertices)
        , m_vertex_stride(vertexStride)
        , m_vertex_buffer_view{}
    {
        create_vertex_buffer_view();
    }

    vertex_buffer::vertex_buffer(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride)
        : buffer(device, resource)
        , m_num_vertices(numVertices)
        , m_vertex_stride(vertexStride)
        , m_vertex_buffer_view{}
    {
        create_vertex_buffer_view();
    }

    vertex_buffer::~vertex_buffer() = default;

    size_t vertex_buffer::get_num_vertices() const
    {
        return m_num_vertices;
    }

    size_t vertex_buffer::get_vertex_stride() const
    {
        return m_vertex_stride;
    }

    D3D12_VERTEX_BUFFER_VIEW vertex_buffer::get_vertex_buffer_view() const
    {
        return m_vertex_buffer_view;
    }

    void vertex_buffer::create_vertex_buffer_view()
    {
        m_vertex_buffer_view.BufferLocation = get_d3d_resource()->GetGPUVirtualAddress();
        m_vertex_buffer_view.SizeInBytes = static_cast<UINT>(m_num_vertices * m_vertex_stride);
        m_vertex_buffer_view.StrideInBytes = static_cast<UINT>(m_vertex_stride);
    }
}