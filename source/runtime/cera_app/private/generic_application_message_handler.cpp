#include "generic_application_message_handler.h"

namespace cera
{
        generic_application_message_handler::~generic_application_message_handler() = default;

        bool generic_application_message_handler::on_key_char(const tchar character, const bool is_repeat)
        {
            return false;
        }

        bool generic_application_message_handler::on_key_down(const s32 key_code, const u32 character_code, const bool is_repeat)
        {
            return false;
        }

        bool generic_application_message_handler::on_key_up(const s32 key_code, const u32 character_code, const bool is_repeat)
        {
            return false;
        }

        bool generic_application_message_handler::on_mouse_down(const std::shared_ptr<generic_window>& window, const mouse_button button)
        {
            return false;
        }

        bool generic_application_message_handler::on_mouse_down(const std::shared_ptr<generic_window>& window, const mouse_button button, const s32 cursor_pos_x, const s32 cursor_pos_y)
        {
            return false;
        }

        bool generic_application_message_handler::on_mouse_up(const mouse_button button)
        {
            return false;
        }

        bool generic_application_message_handler::on_mouse_up(const mouse_button button, const s32 cursor_pos_x, const s32 cursor_pos_y)
        {
            return false;
        }

        bool generic_application_message_handler::on_mouse_double_click(const std::shared_ptr<generic_window>& window, const mouse_button button)
        {
            return false;
        }

        bool generic_application_message_handler::on_mouse_double_click(const std::shared_ptr<generic_window>& window, const mouse_button button, const s32 cursor_pos_x, const s32 cursor_pos_y)
        {
            return false;
        }

        bool generic_application_message_handler::on_mouse_wheel(const f32 Delta)
        {
            return false;
        }

        bool generic_application_message_handler::on_mouse_wheel(const f32 Delta, const s32 cursor_pos_x, const s32 cursor_pos_y)
        {
            return false;
        }

        bool generic_application_message_handler::on_mouse_move()
        {
            return false;
        }

        bool generic_application_message_handler::on_raw_mouse_move(const s32 x, const s32 Y)
        {
            return false;
        }

        bool generic_application_message_handler::on_size_changed(const std::shared_ptr<generic_window>& window, const s32 width, const s32 height, bool was_minimized)
        {
            return false;
        }

        void generic_application_message_handler::on_os_paint(const std::shared_ptr<generic_window>& window)
        {
        }

        void generic_application_message_handler::on_resizing_window(const std::shared_ptr<generic_window>& window)
        {
        }

        bool generic_application_message_handler::begin_reshaping_window(const std::shared_ptr<generic_window>& window)
        {
            return true;
        }

        void generic_application_message_handler::finished_reshaping_window(const std::shared_ptr<generic_window>& window)
        {
        }

        void generic_application_message_handler::on_moved_window(const std::shared_ptr<generic_window>& window, const s32 x, const s32 y)
        {
        }

        bool generic_application_message_handler::on_window_activation_changed(const std::shared_ptr<generic_window>& window, const window_activation activation_type)
        {
            return false;
        }

        bool generic_application_message_handler::on_application_activation_changed(const bool is_active)
        {
            return false;
        }

        bool generic_application_message_handler::on_convertible_laptop_mode_changed()
        {
            return false;
        }

        void generic_application_message_handler::on_window_close(const std::shared_ptr<generic_window>& window)
        {
        }

        bool generic_application_message_handler::on_window_action(const std::shared_ptr<generic_window>& window, const window_action action_type)
        {
            return true;
        }
}