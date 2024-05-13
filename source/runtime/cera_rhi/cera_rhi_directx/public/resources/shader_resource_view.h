#pragma once

#include "util/types.h"

#include "directx_util.h"
#include "descriptors/descriptor_allocation.h"
#include "rhi_resource.h"

namespace cera
{
    namespace renderer
    {
        class Resource;
        class Device;

        class ShaderResourceView : public rhi_resource
        {
        public:
            RESOURCE_CLASS_TYPE(ShaderResourceView);

            std::shared_ptr<Resource> resource() const;

            D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle() const;

        protected:
            ShaderResourceView(Device& device, const std::shared_ptr<Resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr);
            virtual ~ShaderResourceView();

        private:
            Device& m_device;
            std::shared_ptr<Resource> m_resource;
            DescriptorAllocation m_descriptor_allocation;
        };
    }
}