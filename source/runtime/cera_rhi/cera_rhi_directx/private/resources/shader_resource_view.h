#pragma once

#include "util/types.h"

#include "rhi_directx_util.h"
#include "descriptors/descriptor_allocation.h"
#include "resources/rhi_resource.h"

namespace cera
{
    namespace renderer
    {
        class resource;
        class d3d12_device;

        class ShaderResourceView : public rhi_resource
        {
        public:
            RESOURCE_CLASS_TYPE(ShaderResourceView);

            std::shared_ptr<resource> get_resource() const;

            D3D12_CPU_DESCRIPTOR_HANDLE get_descriptor_handle() const;

        protected:
            ShaderResourceView(d3d12_device& device, const std::shared_ptr<resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr);
            virtual ~ShaderResourceView();

        private:
            d3d12_device& m_device;
            std::shared_ptr<resource> m_resource;
            DescriptorAllocation m_descriptor_allocation;
        };
    }
}