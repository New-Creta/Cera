#include "resources/constant_buffer_view.h"
#include "resources/constant_buffer.h"

#include "rhi_directx_device.h"

#include "util/assert.h"
#include "util/pointer_math.h"

namespace cera
{
  namespace renderer
  {
    ConstantBufferView::ConstantBufferView(d3d12_device& device, const std::shared_ptr<ConstantBuffer>& constantBuffer, size_t offset)
        : m_device(device)
        , m_constant_buffer(constantBuffer)
    {
      CERA_ASSERT_X(constantBuffer, "Invalid ConstantBuffer given");

      auto d3d_device   = m_device.d3d_device();
      auto d3d_resource = m_constant_buffer->d3d_resource();

      D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
      cbv.BufferLocation = d3d_resource->GetGPUVirtualAddress() + offset;
      cbv.SizeInBytes    = static_cast<u32>(align_up(m_constant_buffer->size_in_bytes(), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)); // Constant buffers must be aligned for hardware requirements.

      m_descriptor_allocation = device.allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

      d3d_device->CreateConstantBufferView(&cbv, m_descriptor_allocation.descriptor_handle());
    }

    ConstantBufferView::~ConstantBufferView() = default;

    std::shared_ptr<ConstantBuffer> ConstantBufferView::constant_buffer() const
    {
      return m_constant_buffer;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ConstantBufferView::descriptor_handle() const
    {
      return m_descriptor_allocation.descriptor_handle();
    }
  } // namespace renderer
} // namespace cera