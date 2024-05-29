#pragma once

#include "util/types.h"

#include "descriptors/descriptor_allocation.h"

#include "resources/rhi_resource.h"

#include "rhi_directx_util.h"

#include <memory>

namespace cera
{
  namespace renderer
  {
    class ConstantBuffer;
    class d3d12_device;

    class ConstantBufferView : public rhi_resource
    {
    public:
      RESOURCE_CLASS_TYPE(ConstantBufferView);

      std::shared_ptr<ConstantBuffer> constant_buffer() const;

      D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle() const;

    protected:
      ConstantBufferView(d3d12_device& device, const std::shared_ptr<ConstantBuffer>& constantBuffer, size_t offset = 0);
      virtual ~ConstantBufferView();

    private:
      d3d12_device& m_device;
      std::shared_ptr<ConstantBuffer> m_constant_buffer;
      DescriptorAllocation m_descriptor_allocation;
    };
  } // namespace renderer
}