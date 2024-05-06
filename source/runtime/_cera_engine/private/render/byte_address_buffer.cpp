#include "render/byte_address_buffer.h"

namespace cera
{
    byte_address_buffer::byte_address_buffer(device& device, const D3D12_RESOURCE_DESC& resDesc)
        : buffer(device, resDesc)
    {}

    byte_address_buffer::byte_address_buffer(device& device, wrl::ComPtr<ID3D12Resource> resource)
        : buffer(device, resource)
    {}

    byte_address_buffer::~byte_address_buffer() = default;

    size_t byte_address_buffer::get_buffer_size() const
    {
        return m_buffer_size;
    }
}