#pragma once

#include <typeinfo>
#include <string>

#define RESOURCE_CLASS_TYPE(resourceType)                                                                                                                                                                                                                \
    static size_t static_type()                                                                                                                                                                                                                          \
    {                                                                                                                                                                                                                                                    \
        return typeid(resourceType).hash_code();                                                                                                                                                                                                         \
    }                                                                                                                                                                                                                                                    \
    size_t type() const override                                                                                                                                                                                                                         \
    {                                                                                                                                                                                                                                                    \
        return static_type();                                                                                                                                                                                                                            \
    }

namespace cera
{
    class rhi_resource
    {
    public:
        virtual ~rhi_resource();

        /**
         * Set the name of the resource. Useful for debugging purposes.
         * The name of the resource will persist if the underlying D3D12 resource is
         * replaced with SetD3D12Resource.
         */
        void set_resource_name(const std::wstring& name);

        /** Retrieve the name of the resource */
        const std::wstring& get_resource_name() const;

        /** Retrieve the type of the resource */
        virtual size_t type() const = 0;

    private:
        std::wstring m_name;
    };
}