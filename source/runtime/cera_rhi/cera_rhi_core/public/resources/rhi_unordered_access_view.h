#pragma once

#include "resources/rhi_resource.h"

#include <memory>

namespace cera
{
    struct rhi_unordered_access_view_desc
    {

    };

    class rhi_unordered_access_view_builder
    {

    };

    class rhi_unordered_access_view
    {
    public:
        virtual ~rhi_unordered_access_view() = default;

        std::shared_ptr<rhi_resource> get_resource() const;
        std::shared_ptr<rhi_resource> get_counter_resource() const;
    };
}