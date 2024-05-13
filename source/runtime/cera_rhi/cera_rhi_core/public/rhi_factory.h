#pragma once

#include <memory>

namespace cera
{
    class rhi;

    namespace rhi_factory
    {
        /**
        *	Each platform that utilizes dynamic RHIs should implement this function
        *	Called to create the instance of the dynamic RHI.
        */
        std::unique_ptr<rhi> create();
    }
}