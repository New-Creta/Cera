#pragma once

#include "util/types.h"

#include <memory>
#include <unordered_map>

namespace cera
{
    class rhi_viewport;

    class generic_window;

    /** An RHI Representation of a viewport with cached width and height for detecting resizes */
    struct viewport_info
    {
        /** The viewport rendering handle */
        std::shared_ptr<rhi_viewport> viewport_rhi;

        /** The OS Window handle (for recreating the viewport) */
        void* os_window;

        /** The actual width of the viewport */
        u32 width;
        /** The actual height of the viewport */
        u32 height;
        /** The desired width of the viewport */
        u32 desired_width;
        /** The desired height of the viewport */
        u32 desired_height;

        /** Whether or not the viewport is in fullscreen */
        bool fullscreen;

        viewport_info() 
            : viewport_rhi(nullptr)
            , os_window(NULL)
            , width(0)
            , height(0)
            , desired_width(0)
            , desired_height(0)
            , fullscreen(false)
        {
        }
    };

    class gui_renderer
    {
      public:
        void create_viewport(std::shared_ptr<generic_window> window);

        void on_window_destroyed(std::shared_ptr<generic_window> window);
        void on_window_resized(std::shared_ptr<generic_window> window);

    private:
        /** A mapping of SWindows to their RHI implementation */
        std::unordered_map<const generic_window*, viewport_info*> m_window_viewport_map;
    };
} // namespace cera