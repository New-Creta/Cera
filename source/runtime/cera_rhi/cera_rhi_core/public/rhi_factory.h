#pragma once

#include <memory>

namespace cera
{
    namespace renderer
    {
        class rhi;

        enum class feature_level;

        namespace rhi_factory
        {
            /**
            * Check if the requested feature level set is supported on this device
            */
            bool is_supported(feature_level in_feature_level);

            /**
             *	Each platform that utilizes dynamic RHIs should implement this function
             *	Called to create the instance of the dynamic RHI.
             */
            std::unique_ptr<rhi> create();

            /**
             * Construct a new viewport for the active RHI
             */
            std::shared_ptr<rhi_viewport> create_viewport(void* in_window_handle, s32 size_x, s32 size_y, bool is_fullscreen);

            /**
             * Resize a viewport for the active RHI
             */
            void resize_viewport(std::shared_ptr<rhi_viewport> in_viewport, s32 size_x, s32 size_y, bool is_fullscreen);
        } // namespace rhi_factory
    }     // namespace renderer
}