#pragma once

#include "util/types.h"

#include "render/d3dx12_declarations.h"
#include "render/descriptor_allocation.h"

#include <memory>

namespace cera
{
    class resource;
    class device;

    class unordered_access_view
    {
    public:
        std::shared_ptr<resource> get_resource() const;
        std::shared_ptr<resource> get_counter_resource() const;

        D3D12_CPU_DESCRIPTOR_HANDLE get_descriptor_handle();

    protected:
        unordered_access_view(device& device, const std::shared_ptr<resource>& inResource, const std::shared_ptr<resource>& inCounterResource = nullptr, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr);
        virtual ~unordered_access_view();

    private:
        device& m_device;
        std::shared_ptr<resource> m_resource;
        std::shared_ptr<resource> m_counter_resource;
        descriptor_allocation m_descriptor;
    };
}