#pragma once

#include "util/types.h"

#include "directx_util.h"

#include "descriptors/descriptor_allocation.h"

#include "rhi_resource.h"

#include <memory>

namespace cera
{
    namespace renderer
    {
        class Resource;
        class Device;

        class UnorderedAccessView : public rhi_resource
        {
        public:
            RESOURCE_CLASS_TYPE(UnorderedAccessView);

            std::shared_ptr<Resource> resource() const;
            std::shared_ptr<Resource> counter_resource() const;

            D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle() const;

        protected:
            UnorderedAccessView(Device& device, const std::shared_ptr<Resource>& inResource, const std::shared_ptr<Resource>& inCounterResource = nullptr, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr);
            virtual ~UnorderedAccessView();

        private:
            Device& m_device;
            std::shared_ptr<Resource> m_resource;
            std::shared_ptr<Resource> m_counter_resource;
            DescriptorAllocation m_descriptor_allocation;
        };
    }
}