#pragma once

#include "rhi_resource.h"

namespace cera
{
    class rhi_buffer : public rhi_resource
    {
    public:
        ~rhi_buffer() override = default;
    }
}