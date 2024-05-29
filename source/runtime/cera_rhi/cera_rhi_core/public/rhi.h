#pragma once

#include <memory>

namespace cera
{
    namespace renderer
    {
        /**
         * Interface for a rendering hardware interface (RHI), defining methods for creating various types of resources.
         */
        class rhi
        {
          public:
            /**
             * Initialize the rendering hardware interface.
             */
            virtual void initialize() = 0;

            /**
             * Shutdown the rendering hardware interface and release associated resources.
             */
            virtual void shutdown() = 0;
        };
    } // namespace renderer
}