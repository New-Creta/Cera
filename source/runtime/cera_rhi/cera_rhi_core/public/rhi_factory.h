#pragma once

#include <memory>

namespace cera
{
    namespace renderer
    {
        class rhi;

        namespace rhi_factory
        {
            /**
            * Check if the requested feature level set is supported on this device
            */
            bool is_supported();

            /**
             *	Each platform that utilizes dynamic RHIs should implement this function
             *	Called to create the instance of the dynamic RHI.
             */
            std::unique_ptr<rhi> create();
        } // namespace rhi_factory
    }     // namespace renderer
}