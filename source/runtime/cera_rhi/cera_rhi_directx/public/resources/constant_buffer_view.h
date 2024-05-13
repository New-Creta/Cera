#pragma once

#include "util/types.h"

#include "directx_util.h"
#include "descriptors/descriptor_allocation.h"
#include "rhi_resource.h"

#include <memory>

namespace cera
{
  namespace renderer
  {
    class ConstantBuffer;
    class Device;

    class ConstantBufferView : public rhi_resource
    {
    public:
      RESOURCE_CLASS_TYPE(ConstantBufferView);

      std::shared_ptr<ConstantBuffer> constant_buffer() const;

      D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle() const;

    protected:
      ConstantBufferView(Device& device, const std::shared_ptr<ConstantBuffer>& constantBuffer, size_t offset = 0);
      virtual ~ConstantBufferView();

    private:
      Device& m_device;
      std::shared_ptr<ConstantBuffer> m_constant_buffer;
      DescriptorAllocation m_descriptor_allocation;
    };
  } // namespace renderer
}