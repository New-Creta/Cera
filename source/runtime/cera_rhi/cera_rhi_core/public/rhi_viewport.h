#pragma once

namespace cera
{
    namespace renderer
    {
        class rhi_viewport
        {
          public:
            /**
             * Returns access to the platform-specific native resource pointer.  This is designed to be used to provide plugins with access
             * to the underlying resource and should be used very carefully or not at all.
             *
             * @return	The pointer to the native resource or NULL if it not initialized or not supported for this resource type for some reason
             */
            virtual void* get_native_swapchain() const
            {
                return nullptr;
            }

            /**
             * Returns access to the platform-specific native window. This is designed to be used to provide plugins with access
             * to the underlying resource and should be used very carefully or not at all.
             *
             * @return	The pointer to the native resource or NULL if it not initialized or not supported for this resource type for some reason.
             */
            virtual void* get_native_window() const
            {
                return nullptr;
            }
        };
    } // namespace renderer
} // namespace cera