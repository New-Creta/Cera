#include "resources/rhi_resource.h"

namespace cera
{
    //-------------------------------------------------------------------------
    rhi_resource::~rhi_resource() = default;

    //-------------------------------------------------------------------------
    void rhi_resource::set_resource_name(const std::wstring& name)
    {
        m_name = name;
    }

    //-------------------------------------------------------------------------
    const std::wstring& rhi_resource::get_resource_name() const
    {
        return m_name;
    }

}