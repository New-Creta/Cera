#include "generic_window.h"

namespace cera
{

    generic_window::generic_window()
    {
    }

    generic_window::~generic_window()
    {
    }

    void generic_window::reshape_window(s32 x, s32 y, s32 width, s32 height)
    {
        // empty default functionality
    }

    bool generic_window::get_full_screen_info(s32& x, s32& y, s32& width, s32& height) const
    {
        return false;
    }

    void generic_window::move_window_to(s32 x, s32 y)
    {
        // empty default functionality
    }

    void generic_window::bring_to_front(bool force)
    {
        // empty default functionality
    }

    void generic_window::destroy()
    {
        // empty default functionality
    }

    void generic_window::minimize()
    {
        // empty default functionality
    }

    void generic_window::maximize()
    {
        // empty default functionality
    }

    void generic_window::restore()
    {
        // empty default functionality
    }

    void generic_window::show()
    {
        // empty default functionality
    }

    void generic_window::hide()
    {
        // empty default functionality
    }

    void generic_window::set_window_mode(window_mode new_window_mode)
    {
        // empty default functionality
    }

    window_mode generic_window::get_window_mode() const
    {
        // default functionality
        return window_mode::windowed;
    }

    bool generic_window::is_maximized() const
    {
        // empty default functionality
        return true;
    }

    bool generic_window::is_minimized() const
    {
        return false;
    }

    bool generic_window::is_visible() const
    {
        // empty default functionality
        return true;
    }

    bool generic_window::get_restored_dimensions(s32& x, s32& y, s32& width, s32& height)
    {
        return false;
    }

    void generic_window::set_window_focus()
    {
        // empty default functionality
    }

    void generic_window::set_opacity(const float in_opacity)
    {
        // empty default functionality
    }

    void generic_window::enable(bool enable)
    {
        // empty default functionality
    }

    bool generic_window::is_point_in_window(s32 x, s32 y) const
    {
        // empty default functionality
        return true;
    }

    s32 generic_window::get_window_border_size() const
    {
        // empty default functionality
        return 0;
    }

    s32 generic_window::get_window_title_bar_size() const
    {
        // empty default functionality
        return 0;
    }

    void *generic_window::get_os_window_handle() const
    {
        return nullptr;
    }

    bool generic_window::is_foreground_window() const
    {
        // empty default functionality
        return true;
    }

    bool generic_window::is_fullscreen_supported() const
    {
        // empty default functionality
        return true;
    }

    void generic_window::set_text(const tchar* const text)
    {
        // empty default functionality
    }

    const generic_window_definition& generic_window::get_definition() const
    {
        return m_definition;
    }

    
    s32 get_window_width() const
    {
        return 
    }

    s32 get_window_height() const
    {

    }

    void generic_window::set_definition(const generic_window_definition& definition)
    {
        m_definition = definition;
    }
}
