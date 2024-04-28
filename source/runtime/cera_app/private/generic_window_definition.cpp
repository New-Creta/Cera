#include "generic_window_definition.h"

namespace cera
{
    generic_window_definition_builder::generic_window_definition_builder()
        : m_window_definiiton()
    {}

    // Methods to set various properties of the window
    generic_window_definition_builder& generic_window_definition_builder::set_type(window_type type) 
    {
         m_window_definiiton.type = type; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_x_position(f32 x) 
    {
         m_window_definiiton.x_desired_position_on_screen = x; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_y_position(f32 y) 
    {
         m_window_definiiton.y_desired_position_on_screen = y; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_width(f32 width) 
    {
         m_window_definiiton.width_desired_on_screen = width; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_height(f32 height) 
    {
         m_window_definiiton.height_desired_on_screen = height; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_has_os_window_border(bool hasBorder) 
    {
         m_window_definiiton.has_os_window_border = hasBorder; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_appears_in_taskbar(bool appears) 
    {
         m_window_definiiton.appears_in_taskbar = appears; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_is_topmost_window(bool isTopmost) 
    {
         m_window_definiiton.is_topmost_window = isTopmost; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_accepts_input(bool accepts) 
    {
         m_window_definiiton.accepts_input = accepts; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_activation_policy(window_activation_policy policy) 
    {
         m_window_definiiton.activation_policy = policy; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_center_behaviour(window_center_behaviour behaviour)
    {
          m_window_definiiton.center_behaviour = behaviour;
          return *this;
    }
    generic_window_definition_builder& generic_window_definition_builder::set_focus_when_first_shown(bool focus) 
    {
         m_window_definiiton.focus_when_first_shown = focus; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_has_close_button(bool hasClose) 
    {
         m_window_definiiton.has_close_button = hasClose; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_supports_minimize(bool supports) 
    {
         m_window_definiiton.supports_minimize = supports; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_supports_maximize(bool supports) 
    {
         m_window_definiiton.supports_maximize = supports; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_is_modal_window(bool isModal) 
    {
         m_window_definiiton.is_modal_window = isModal; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_is_regular_window(bool isRegular) 
    {
         m_window_definiiton.is_regular_window = isRegular; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_has_sizing_frame(bool hasSizingFrame) 
    {
         m_window_definiiton.has_sizing_frame = hasSizingFrame; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_preserve_aspect_ratio(bool preserve) 
    {
         m_window_definiiton.should_preserve_aspect_ratio = preserve; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_expected_max_width(s32 maxWidth) 
    {
         m_window_definiiton.expected_max_width = maxWidth; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_expected_max_height(s32 maxHeight) 
    {
         m_window_definiiton.expected_max_height = maxHeight; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_title(const std::wstring& title) 
    {
         m_window_definiiton.title = title; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_opacity(f32 opacity) 
    {
         m_window_definiiton.opacity = opacity; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_corner_radius(s32 cornerRadius) 
    {
         m_window_definiiton.corner_radius = cornerRadius; 
         return *this; 
    }
    generic_window_definition_builder& generic_window_definition_builder::set_size_limits(const window_size_limits& limits) 
    {
         m_window_definiiton.size_limits = limits; 
         return *this; 
    }

    // Method to build the window
    generic_window_definition generic_window_definition_builder::build() const 
    {
         return m_window_definiiton; 
    }
}