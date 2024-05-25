#pragma once

/**
 *  @brief A wrapper for a DX12 resource. This provides a base class for all
 *  other resource types (Buffers & Textures).
 */

#include "util/types.h"
#include "util/windows_types.h"

#include "resources/rhi_resource.h"

#include "directx_util.h"

namespace cera
{
    namespace renderer
    {
        class Device;

        class Resource : public rhi_resource
        {
        public:
            // Get the Device that was used to create this resource
            Device* device() const;

            // Get access to the underlying D3D12 resource
            wrl::ComPtr<ID3D12Resource> d3d_resource() const;

            // Get access to the underlying D3D12 resource description
            D3D12_RESOURCE_DESC d3d_resource_desc() const;

            /**
             * Set the name of the resource. 
             * Useful for debugging purposes.
             */
            void set_resource_name(const std::wstring& name);

            /**
            * Check if the resource format supports a specific feature.
            */
            bool check_format_support(D3D12_FORMAT_SUPPORT1 formatSupport) const;
            bool check_format_support(D3D12_FORMAT_SUPPORT2 formatSupport) const;

        protected:
            // Resource creation should go through the Device.
            Resource(Device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr);
            Resource(Device& device, wrl::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);

            virtual ~Resource();

            const D3D12_CLEAR_VALUE* d3d_clear_value() const;

            const std::wstring& resource_name() const;

        private:
            // Check the format support and populate the m_format_support structure.
            bool check_feature_support();

        private:
            Device&                             m_device_ref;
            wrl::ComPtr<ID3D12Resource>         m_d3d_resource;
            std::unique_ptr<D3D12_CLEAR_VALUE>  m_d3d_clear_value;
            D3D12_FEATURE_DATA_FORMAT_SUPPORT   m_format_support;
            std::wstring                        m_resource_name;
        };
    }
}