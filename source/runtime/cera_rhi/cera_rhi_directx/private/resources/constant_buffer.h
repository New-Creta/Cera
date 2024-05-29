#pragma once

#include "rhi_directx_util.h"
#include "resources/buffer.h"

namespace cera
{
  namespace renderer
  {
    class ConstantBuffer : public Buffer
    {
    public:
      RESOURCE_CLASS_TYPE(ConstantBuffer);

      size_t size_in_bytes() const;

    protected:
      ConstantBuffer(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource);
      ~ConstantBuffer() override;

    private:
      size_t m_size_in_bytes;
    };
  } // namespace renderer
}