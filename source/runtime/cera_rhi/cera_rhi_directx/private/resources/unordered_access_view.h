#pragma once

#include "util/types.h"

#include "rhi_directx_util.h"

#include "descriptors/descriptor_allocation.h"

#include "resources/rhi_resource.h"

#include <memory>

namespace cera
{
    namespace renderer
    {
        class resource;
        class d3d12_device;

        class UnorderedAccessView : public rhi_resource
        {
        public:
            RESOURCE_CLASS_TYPE(UnorderedAccessView);

            std::shared_ptr<resource> get_resource() const;
            std::shared_ptr<resource> get_counter_resource() const;

            D3D12_CPU_DESCRIPTOR_HANDLE get_descriptor_handle() const;

        protected:
            UnorderedAccessView(d3d12_device& device, const std::shared_ptr<resource>& inResource, const std::shared_ptr<resource>& inCounterResource = nullptr, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr);
            virtual ~UnorderedAccessView();

        private:
            d3d12_device& m_device;
            std::shared_ptr<resource> m_resource;
            std::shared_ptr<resource> m_counter_resource;
            DescriptorAllocation m_descriptor_allocation;
        };
    }
}