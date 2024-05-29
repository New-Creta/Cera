#include "resources/vertex_buffer.h"

namespace cera
{
  namespace renderer
  {
      VertexBuffer::VertexBuffer(d3d12_device& device, size_t num_vertices, size_t vertex_stride)
          : Buffer(device, CD3DX12_RESOURCE_DESC::Buffer(num_vertices * vertex_stride)), m_num_vertices(num_vertices),
            m_vertex_stride(vertex_stride), m_vertex_buffer_view{}
      {
          create_vertex_buffer_view();
    }

    VertexBuffer::VertexBuffer(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource, size_t num_vertices, size_t vertex_stride)
        : Buffer(device, resource), m_num_vertices(num_vertices), m_vertex_stride(vertex_stride), m_vertex_buffer_view{}
    {
      create_vertex_buffer_view();
    }

    VertexBuffer::~VertexBuffer() = default;

    size_t VertexBuffer::num_vertices() const
    {
      return m_num_vertices;
    }

    size_t VertexBuffer::vertex_stride() const
    {
      return m_vertex_stride;
    }

    D3D12_VERTEX_BUFFER_VIEW VertexBuffer::vertex_buffer_view() const
    {
      return m_vertex_buffer_view;
    }

    void VertexBuffer::create_vertex_buffer_view()
    {
      m_vertex_buffer_view.BufferLocation = d3d_resource()->GetGPUVirtualAddress();
      m_vertex_buffer_view.SizeInBytes    = static_cast<UINT>(m_num_vertices * m_vertex_stride);
      m_vertex_buffer_view.StrideInBytes  = static_cast<UINT>(m_vertex_stride);
    }
  } // namespace renderer
} // namespace cera