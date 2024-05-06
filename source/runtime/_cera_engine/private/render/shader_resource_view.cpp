#include "render/shader_resource_view.h"
#include "render/resource.h"
#include "render/device.h"

#include "util/memory_helpers.h"

namespace cera
{
    shader_resource_view::shader_resource_view(device& device, const std::shared_ptr<resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
        : m_device(device)
        , m_resource(resource)
    {
        assert(resource || srv);

        auto d3d_resource = m_resource ? m_resource->get_d3d_resource() : nullptr;
        auto d3d_device = m_device.get_d3d_device();

        m_descriptor = m_device.allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        d3d_device->CreateShaderResourceView(d3d_resource.Get(), srv, m_descriptor.get_descriptor_handle());
    }

    shader_resource_view::~shader_resource_view() = default;

    std::shared_ptr<resource> shader_resource_view::get_resource() const
    {
        return m_resource;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE shader_resource_view::get_descriptor_handle()
    {
        return m_descriptor.get_descriptor_handle();
    }
}