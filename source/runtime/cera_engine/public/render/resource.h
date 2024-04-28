#pragma once

/**
 *  @brief A wrapper for a DX12 resource. This provides a base class for all
 *  other resource types (Buffers & Textures).
 */

#include "util/types.h"

#include "device/windows_types.h"

#include "render/d3dx12_declarations.h"

namespace cera
{
    class device;

    class resource
    {
    public:
        // Get the device that was used to create this resource
        device* get_device() const;

        // Get access to the underlying D3D12 resource
        wrl::ComPtr<ID3D12Resource> get_d3d_resource() const;

        // Get access to the underlying D3D12 resource description
        D3D12_RESOURCE_DESC get_d3d_resource_desc() const;

        /**
         * Set the name of the resource. Useful for debugging purposes.
         * The name of the resource will persist if the underlying D3D12 resource is
         * replaced with SetD3D12Resource.
         */
        void set_resource_name(const std::wstring& name);

        /**
        * Check if the resource format supports a specific feature.
        */
        bool check_format_support(D3D12_FORMAT_SUPPORT1 formatSupport) const;
        bool check_format_support(D3D12_FORMAT_SUPPORT2 formatSupport) const;

    protected:
        // Resource creation should go through the device.
        resource(device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr);
        resource(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);

        virtual ~resource();

        D3D12_CLEAR_VALUE* get_d3d_clear_value() const;

        const std::wstring& get_resource_name() const;

    private:
        // Check the format support and populate the m_FormatSupport structure.
        bool check_feature_support();

    private:
        device& m_device_ref;
        wrl::ComPtr<ID3D12Resource> m_d3d_resource;
        D3D12_FEATURE_DATA_FORMAT_SUPPORT m_format_support;
        std::unique_ptr<D3D12_CLEAR_VALUE> m_d3d_clear_value;
        std::wstring m_resource_name;
    };
}