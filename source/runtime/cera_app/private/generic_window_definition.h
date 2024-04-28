#pragma once

#include "util/types.h"

#include <optional>
#include <string>

namespace cera
{
    /** Enumeration to specify different window types for SWindows */
    enum class window_type
    {
        /** Value indicating that this is a standard, general-purpose window */
        normal,
        /** Value indicating that this is a window used for a tooltip */
        tool_tip,
        /** Value indicating that this is a game window */
        game_window
    };

    /** Enumeration to specify whether the window gets activated upon showing it */
    enum class window_activation_policy
    {
        /** Value indicating that a window never activates when it is shown */
        never,

        /** Value indicating that a window always activates when it is shown */
        always,

        /** Value indicating that a window only activates when it is first shown */
        first_shown
    };

    enum class window_center_behaviour
    {
        /** Don't auto-center the window */
        none,

        /** Auto-center the window on the primary work area */
        auto_center,
    };

    /** Defines the minimum and maximum dimensions that a window can take on. */
    struct window_size_limits
    {
    public:
        window_size_limits &set_min_width(std::optional<f32> InValue)
        {
            m_min_width = InValue;
            return *this;
        }
        const std::optional<f32> &get_min_width() const { return m_min_width; }

        window_size_limits &set_min_height(std::optional<f32> InValue)
        {
            m_min_height = InValue;
            return *this;
        }
        const std::optional<f32> &get_min_height() const { return m_min_height; }

        window_size_limits &set_max_width(std::optional<f32> InValue)
        {
            m_max_width = InValue;
            return *this;
        }
        const std::optional<f32> &get_max_width() const { return m_max_width; }

        window_size_limits &set_max_height(std::optional<f32> InValue)
        {
            m_max_height = InValue;
            return *this;
        }
        const std::optional<f32> &get_max_height() const { return m_max_height; }

    private:
        std::optional<f32> m_min_width = { (f32)1 };
        std::optional<f32> m_min_height = { (f32)1 };
        std::optional<f32> m_max_width = { (f32)std::numeric_limits<s16>::max() };
        std::optional<f32> m_max_height = { (f32)std::numeric_limits<s16>::max() };
    };

    struct generic_window_definition
    {
        /** Window type */
        window_type type = window_type::normal;

        /** The initially desired horizontal screen position */
        f32 x_desired_position_on_screen = 0.0f;
        /** The initially desired vertical screen position */
        f32 y_desired_position_on_screen = 0.0f;

        /** The initially desired width */
        f32 width_desired_on_screen = 0.0f;
        /** The initially desired height */
        f32 height_desired_on_screen = 0.0f;

        /** true if the window is using the os window border instead of a slate created one */
        bool has_os_window_border = false;
        /** should this window show up in the taskbar */
        bool appears_in_taskbar = false;
        /** true if the window should be on top of all other windows; false otherwise */
        bool is_topmost_window = false;
        /** true if the window accepts input; false if the window is non-interactive */
        bool accepts_input = false;
        /** the policy for activating the window upon each show */
        window_activation_policy activation_policy = window_activation_policy::always;
        /** the behaviour of what should happen when calculating the position of the window */
        window_center_behaviour center_behaviour = window_center_behaviour::auto_center;
        /** true if this window will be focused when it is first shown */
        bool focus_when_first_shown = true;
        /** true if this window displays an enabled close button on the toolbar area */
        bool has_close_button = false;
        /** true if this window displays an enabled minimize button on the toolbar area */
        bool supports_minimize = false;
        /** true if this window displays an enabled maximize button on the toolbar area */
        bool supports_maximize = false;

        /** true if the window is modal (prevents interacting with its parent) */
        bool is_modal_window = false;
        /** true if this is a vanilla window, or one being used for some special purpose: e.g. tooltip or menu */
        bool is_regular_window = false;
        /** true if this is a user-sized window with a thick edge */
        bool has_sizing_frame = false;
        /** true if the window should preserve its aspect ratio when resized by user */
        bool should_preserve_aspect_ratio = false;
        /** The expected maximum width of the window.  May be used for performance optimization when SizeWillChangeOften is set. */
        s32 expected_max_width = -1;
        /** The expected maximum height of the window.  May be used for performance optimization when SizeWillChangeOften is set. */
        s32 expected_max_height = -1;

        /** the title of the window */
        std::wstring title = L"Generic Window";
        /** opacity of the window (0-1) */
        f32 opacity = 1.0f;
        /** the radius of the corner rounding of the window */
        s32 corner_radius = 2;

        window_size_limits size_limits = {};
    };

    class generic_window_definition_builder
    {
    public:
        generic_window_definition_builder();

        generic_window_definition_builder& set_type(window_type type);
        generic_window_definition_builder& set_x_position(f32 x);
        generic_window_definition_builder& set_y_position(f32 y);
        generic_window_definition_builder& set_width(f32 width);
        generic_window_definition_builder& set_height(f32 height);
        generic_window_definition_builder& set_has_os_window_border(bool hasBorder);
        generic_window_definition_builder& set_appears_in_taskbar(bool appears);
        generic_window_definition_builder& set_is_topmost_window(bool isTopmost);
        generic_window_definition_builder& set_accepts_input(bool accepts);
        generic_window_definition_builder& set_activation_policy(window_activation_policy policy);
        generic_window_definition_builder& set_center_behaviour(window_center_behaviour behaviour);
        generic_window_definition_builder& set_focus_when_first_shown(bool focus);
        generic_window_definition_builder& set_has_close_button(bool hasClose);
        generic_window_definition_builder& set_supports_minimize(bool supports);
        generic_window_definition_builder& set_supports_maximize(bool supports);
        generic_window_definition_builder& set_is_modal_window(bool isModal);
        generic_window_definition_builder& set_is_regular_window(bool isRegular);
        generic_window_definition_builder& set_has_sizing_frame(bool hasSizingFrame);
        generic_window_definition_builder& set_preserve_aspect_ratio(bool preserve);
        generic_window_definition_builder& set_expected_max_width(s32 maxWidth);
        generic_window_definition_builder& set_expected_max_height(s32 maxHeight);
        generic_window_definition_builder& set_title(const std::wstring& title);
        generic_window_definition_builder& set_opacity(f32 opacity);
        generic_window_definition_builder& set_corner_radius(s32 cornerRadius);
        generic_window_definition_builder& set_size_limits(const window_size_limits& limits);

        generic_window_definition build() const;

    private:
        generic_window_definition m_window_definiiton;
    };
}