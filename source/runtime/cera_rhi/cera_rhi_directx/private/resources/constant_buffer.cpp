#include "resources/constant_buffer.h"

namespace cera
{
  namespace renderer
  {
    ConstantBuffer::ConstantBuffer(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource)
        : Buffer(device, resource)
    {
      m_size_in_bytes = d3d_resource_desc().Width;
    }

    ConstantBuffer::~ConstantBuffer() = default;

    size_t ConstantBuffer::size_in_bytes() const
    {
      return m_size_in_bytes;
    }
  } // namespace renderer
} // namespace cera