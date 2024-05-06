#pragma once

#include <string>

namespace cera
{
    class rhi_resource
    {
    public:
        virtual ~rhi_resource() = default;

        /**
         * Set the name of the resource. Useful for debugging purposes.
         * The name of the resource will persist if the underlying D3D12 resource is
         * replaced with SetD3D12Resource.
         */
        virtual void set_resource_name(const std::wstring& name) = 0;

        /** Retrieve the name of the resource */
        virtual const std::wstring& get_resource_name() const = 0;
    };
}