#include "render/index_buffer.h"

namespace cera
{
    index_buffer::index_buffer(device& device, size_t numIndices, DXGI_FORMAT indexFormat)
        : buffer(device, CD3DX12_RESOURCE_DESC::Buffer(numIndices* (indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4)))
        , m_num_indicies(numIndices)
        , m_index_format(indexFormat)
        , m_index_buffer_view{}
    {
        assert(indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT);
        create_index_buffer_view();
    }

    index_buffer::index_buffer(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat)
        : buffer(device, resource)
        , m_num_indicies(numIndices)
        , m_index_format(indexFormat)
        , m_index_buffer_view{}
    {
        assert(indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT);
        create_index_buffer_view();
    }

    index_buffer::~index_buffer() = default;

    size_t index_buffer::get_num_indices() const
    {
        return m_num_indicies;
    }

    DXGI_FORMAT index_buffer::get_index_format() const
    {
        return m_index_format;
    }

    D3D12_INDEX_BUFFER_VIEW index_buffer::get_index_buffer_view() const
    {
        return m_index_buffer_view;
    }

    void index_buffer::create_index_buffer_view()
    {
        u32 buffer_size = m_num_indicies * (m_index_format == DXGI_FORMAT_R16_UINT ? 2 : 4);

        m_index_buffer_view.BufferLocation = get_d3d_resource()->GetGPUVirtualAddress();
        m_index_buffer_view.SizeInBytes = buffer_size;
        m_index_buffer_view.Format = m_index_format;
    }
}