#include "resources/byte_address_buffer.h"

namespace cera
{
  namespace renderer
  {
      ByteAddressBuffer::ByteAddressBuffer(d3d12_device& device, const D3D12_RESOURCE_DESC& res_desc)
          : Buffer(device, res_desc)
      {
    }

    ByteAddressBuffer::ByteAddressBuffer(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource)
        : Buffer(device, resource)
    {
    }

    ByteAddressBuffer::~ByteAddressBuffer() = default;

    size_t ByteAddressBuffer::buffer_size() const
    {
      return m_buffer_size;
    }
  } // namespace renderer
} // namespace cera