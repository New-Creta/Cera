#pragma once

#include "common/rhi_format.h"

#include "dxgi/dxgi_util.h"

namespace cera
{
    namespace renderer
    {
        namespace conversions
        {
            DXGI_FORMAT to_dxgi_format_format(renderer::Format format);
        }
    }
}