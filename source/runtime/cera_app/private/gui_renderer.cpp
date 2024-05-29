#include "gui_renderer.h"

#include "generic_window.h"

#include "core_globals.h"
#include "core_platform.h"

#include "rhi_factory.h"

#include "util/assert.h"

namespace cera
{
    void gui_renderer::create_viewport(std::shared_ptr<generic_window> window)
    {
        if (m_window_viewport_map.find(window.get()) == std::cend(m_window_viewport_map))
        {
            // Clamp the window size to a reasonable default anything below 8 is a d3d warning and 8 is used anyway.
            s32 window_width = window->get_window_width();
            s32 window_height = window->get_window_height();

            // Sanity check dimensions
            CERA_ASSERT_X(window_width >= g_minimum_window_width, "Invalid window width={0}", window_width);
            CERA_ASSERT_X(window_height >= g_minimum_window_height, "Invalid window height={0}", window_height);

            std::unique_ptr<viewport_info> new_info = std::make_unique<viewport_info>();
            
            new_info->os_window = window->get_os_window_handle();
            new_info->width = window_width;
            new_info->height = window_height;
            new_info->desired_width = window_width;
            new_info->desired_height = window_height;

            bool is_fullscreen = is_viewport_fullscreen(window);

            new_info->viewport_rhi = renderer::rhi_factory::create_viewport(new_info->os_window, window_width, window_height, is_fullscreen);
            new_info->fullscreen = is_fullscreen;

            m_window_viewport_map.emplace(window.get(), new_info);
        }
    }

    void gui_renderer::on_window_destroyed(std::shared_ptr<generic_window> window)
    {

    }

    void gui_renderer::on_window_resized(std::shared_ptr<generic_window> window)
    {

    }

    bool gui_renderer::is_viewport_fullscreen(std::shared_ptr<generic_window> window)
    {
        bool fullscreen = false;

        if (platform::supports_window_mode())
        {
            fullscreen = window->is_fullscreen_supported() && window->get_window_mode() == window_mode::fullscreen;

#if CERA_PLATFORM_WINDOWS
            // When we are in fullscreen mode but the user alt-tabs out we need to temporarily drop out of fullscreen while the window has lost focus, otherwise DXGI will eventually
            // forcibly throw us out of fullscreen mode with device loss and crash as typical result. By returning false here we'll trigger a mode switch to windowed when the user
            // alt-tabs, and back to fullscreen again once the window comes back in focus, through the regular path. DXGI will never need to intervene and everyone is happy.
            fullscreen = fullscreen && window->is_foreground_window();
#endif
        }
        else
        {
            fullscreen = true;
        }

        return fullscreen;
    }
}