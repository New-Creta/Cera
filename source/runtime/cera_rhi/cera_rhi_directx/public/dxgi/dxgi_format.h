#pragma once

#include "rhi_format.h"
#include "dxgi/dxgi_util.h"

namespace cera
{
    namespace dxgi
    {
        namespace conversions
        {
            DXGI_FORMAT to_DXGI(renderer::Format format);
        }
    }
}