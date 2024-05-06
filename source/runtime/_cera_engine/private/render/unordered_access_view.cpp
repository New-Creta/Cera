#include "render/unordered_access_view.h"
#include "render/resource.h"
#include "render/device.h"

#include "util/memory_helpers.h"

namespace cera
{
    unordered_access_view::unordered_access_view(device& device, const std::shared_ptr<resource>& inResource, const std::shared_ptr<resource>& inCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
        : m_device(device)
        , m_resource(inResource)
        , m_counter_resource(inCounterResource)
    {
        assert(m_resource || uav);

        auto d3d_device = m_device.get_d3d_device();
        auto d3d_resource = m_resource ? m_resource->get_d3d_resource() : nullptr;
        auto d3d_counter_resource = m_counter_resource ? m_counter_resource->get_d3d_resource() : nullptr;

        if (m_resource)
        {
            auto d3d_resource_desc = m_resource->get_d3d_resource_desc();

            // Resource must be created with the D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS flag.
            assert((d3d_resource_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0);
        }

        m_descriptor = m_device.allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        d3d_device->CreateUnorderedAccessView(d3d_resource.Get(), d3d_counter_resource.Get(), uav, m_descriptor.get_descriptor_handle());
    }

    unordered_access_view::~unordered_access_view() = default;

    std::shared_ptr<resource> unordered_access_view::get_resource() const
    {
        return m_resource;
    }

    std::shared_ptr<resource> unordered_access_view::get_counter_resource() const
    {
        return m_counter_resource;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE unordered_access_view::get_descriptor_handle()
    {
        return m_descriptor.get_descriptor_handle();
    }
}