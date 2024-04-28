#include "render/constant_buffer.h"

namespace cera
{
    constant_buffer::constant_buffer(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource)
        : buffer(device, resource)
    {
        m_size_in_bytes = get_d3d_resource_desc().Width;
    }

    constant_buffer::~constant_buffer() = default;

    size_t constant_buffer::get_size_in_bytes() const
    {
        return m_size_in_bytes;
    }
}