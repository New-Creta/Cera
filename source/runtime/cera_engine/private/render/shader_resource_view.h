#pragma once

#include "util/types.h"

#include "render/d3dx12_declarations.h"
#include "render/descriptor_allocation.h"

#include <memory>

namespace cera
{
    class resource;
    class device;

    class shader_resource_view
    {
    public:
        std::shared_ptr<resource> get_resource() const;

        D3D12_CPU_DESCRIPTOR_HANDLE get_descriptor_handle();

    protected:
        shader_resource_view(device& device, const std::shared_ptr<resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr);
        virtual ~shader_resource_view();

    private:
        device& m_device;
        std::shared_ptr<resource> m_resource;
        descriptor_allocation m_descriptor;
    };
}