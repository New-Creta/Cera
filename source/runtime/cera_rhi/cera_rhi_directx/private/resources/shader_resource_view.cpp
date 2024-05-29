#include "resources/shader_resource_view.h"
#include "resources/resource.h"

#include "rhi_directx_device.h"

#include "util/assert.h"

namespace cera
{
  namespace renderer
  {
    ShaderResourceView::ShaderResourceView(d3d12_device& device, const std::shared_ptr<resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
        : m_device(device)
        , m_resource(resource)
    {
      CERA_ASSERT_X(resource || srv, "Invalid resource was given to ShaderResourceView");

      auto d3d_resource = m_resource ? m_resource->d3d_resource() : nullptr;
      auto d3d_device   = m_device.d3d_device();

      m_descriptor_allocation = m_device.allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

      d3d_device->CreateShaderResourceView(d3d_resource.Get(), srv, m_descriptor_allocation.descriptor_handle());
    }

    ShaderResourceView::~ShaderResourceView() = default;

    std::shared_ptr<resource> ShaderResourceView::get_resource() const
    {
      return m_resource;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ShaderResourceView::get_descriptor_handle() const
    {
      return m_descriptor_allocation.descriptor_handle();
    }
  } // namespace renderer
} // namespace cera