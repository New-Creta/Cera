#pragma once

#include "directx_util.h"
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
      ConstantBuffer(Device& device, wrl::ComPtr<ID3D12Resource> resource);
      ~ConstantBuffer() override;

    private:
      size_t m_size_in_bytes;
    };
  } // namespace renderer
}