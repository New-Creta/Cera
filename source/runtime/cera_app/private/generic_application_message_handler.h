#pragma once

#include "util/types.h"

#include <memory>

namespace cera
{
    class generic_window;

    enum class mouse_button
    {
        left = 0,
        middle,
        right,

        thumb01,
        thumb02,

        invalid
    };

    enum class window_activation
    {
        activate,
        activate_by_mouse,
        deactivate
    };

    enum class window_action
    {
        clicked_non_client_area = 1,
        maximize = 2,
        restore = 3,
        window_menu = 4,
    };

    class generic_application_message_handler
    {
    public:
        virtual ~generic_application_message_handler();

        virtual bool on_key_char(const tchar character, const bool is_repeat);

        virtual bool on_key_down(const s32 key_code, const u32 character_code, const bool is_repeat);

        virtual bool on_key_up(const s32 key_code, const u32 character_code, const bool is_repeat);

        virtual bool on_mouse_down(const std::shared_ptr<generic_window>& window, const mouse_button button);

        virtual bool on_mouse_down(const std::shared_ptr<generic_window>& window, const mouse_button button, const s32 cursor_pos_x, const s32 cursor_pos_y);

        virtual bool on_mouse_up(const mouse_button button);

        virtual bool on_mouse_up(const mouse_button button, const s32 cursor_pos_x, const s32 cursor_pos_y);

        virtual bool on_mouse_double_click(const std::shared_ptr<generic_window>& window, const mouse_button button);

        virtual bool on_mouse_double_click(const std::shared_ptr<generic_window>& window, const mouse_button button, const s32 cursor_pos_x, const s32 cursor_pos_y);

        virtual bool on_mouse_wheel(const f32 delta);

        virtual bool on_mouse_wheel(const f32 delta, const s32 cursor_pos_x, const s32 cursor_pos_y);

        virtual bool on_mouse_move();

        virtual bool on_raw_mouse_move(const s32 x, const s32 Y);

        virtual bool on_size_changed(const std::shared_ptr<generic_window>& window, const s32 width, const s32 height, bool was_minimized);

        virtual void on_os_paint(const std::shared_ptr<generic_window>& window);

        virtual void on_resizing_window(const std::shared_ptr<generic_window>& window);

        virtual bool begin_reshaping_window(const std::shared_ptr<generic_window>& window);

        virtual void finished_reshaping_window(const std::shared_ptr<generic_window>& window);

        virtual void on_moved_window(const std::shared_ptr<generic_window>& window, const s32 x, const s32 y);

        virtual bool on_window_activation_changed(const std::shared_ptr<generic_window>& window, const window_activation activation_type);

        virtual bool on_application_activation_changed(const bool is_active);

        virtual bool on_convertible_laptop_mode_changed();

        virtual void on_window_close(const std::shared_ptr<generic_window>& window);

        virtual bool on_window_action(const std::shared_ptr<generic_window>& window, const window_action action_type);
    };
}