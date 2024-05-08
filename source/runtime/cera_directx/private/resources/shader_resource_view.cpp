#include "resources/shader_resource_view.h"
#include "resources/resource.h"

#include "directx_device.h"

#include "cera_engine/diagnostics/assert.h"

namespace cera
{
  namespace renderer
  {
    ShaderResourceView::ShaderResourceView(Device& device, const std::shared_ptr<Resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
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

    std::shared_ptr<Resource> ShaderResourceView::resource() const
    {
      return m_resource;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ShaderResourceView::descriptor_handle() const
    {
      return m_descriptor_allocation.descriptor_handle();
    }
  } // namespace renderer
} // namespace cera