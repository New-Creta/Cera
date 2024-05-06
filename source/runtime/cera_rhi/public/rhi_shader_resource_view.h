#pragma once

#include "rhi_resource.h"

#include <memory>

namespace cera
{
    struct rhi_shader_resource_view_desc
    {

    };

    class rhi_shader_resource_view_desc_builder
    {

    };

    class rhi_shader_resource_view
    {
    public:
        virtual ~rhi_shader_resource_view() = default;

        std::shared_ptr<rhi_resource> get_resource() const;
    };
}