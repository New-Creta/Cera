#pragma once

#include "util/types.h"

#include "key_codes.h"

#include <memory>

namespace cera
{
    class command_list;

    namespace events
    {
        class args
        {
        public:
            args() = default;
        };

        class resize_args : public args
        {
        public:
            resize_args(s32 w, s32 h)
                : width(w)
                , height(h)
            {}

            s32 width;
            s32 height;
        };

        class update_args : public args
        {
        public:
            update_args(u64 fDeltaTime, u64 fTotalTime)
                : elapsed_time(fDeltaTime)
                , total_time(fTotalTime)
            {}

            u64 elapsed_time;
            u64 total_time;
        };

        class render_args : public args
        {
        public:
            render_args(u64 fDeltaTime, u64 fTotalTime, const std::shared_ptr<command_list>& commandList)
                : elapsed_time(fDeltaTime)
                , total_time(fTotalTime)
                , command_list(commandList)
            {}

            u64 elapsed_time;
            u64 total_time;

            std::shared_ptr<command_list> command_list;
        };

        class render_gui_args : public args
        {
        public:
            render_gui_args(u64 fDeltaTime, u64 fTotalTime)
                : elapsed_time(fDeltaTime)
                , total_time(fTotalTime)
            {}

            u64 elapsed_time;
            u64 total_time;
        };

        class key_args : public args
        {
        public:
            enum class key_state
            {
                Released = 0,
                Pressed = 1
            };

            key_args(key_code::Key key, u32 c, key_state state, bool control, bool shift, bool alt)
                :key(key)
                ,character(c)
                ,state(state)
                ,control(control)
                ,shift(shift)
                ,alt(alt)
            {}

            key_code::Key key;        // The Key Code that was pressed or released.
            u32 character;           // The 32-bit character code that was pressed. This value will be 0 if it is a non-prs32able character.
            key_state state;          // Was the key pressed or released?
            bool control;            // Is the Control modifier pressed
            bool shift;              // Is the Shift modifier pressed
            bool alt;                // Is the Alt modifier pressed
        };

        class mouse_motion_args : public args
        {
        public:
            mouse_motion_args(bool left_button, bool middle_button, bool right_button, bool control, bool shift, s32 x, s32 y)
                :left_button(left_button)
                ,middle_button(middle_button)
                ,right_button(right_button)
                ,control(control)
                ,shift(shift)
                ,x(x)
                ,y(y)
                ,rel_x(0)
                ,rel_y(0)
            {}

            bool left_button;        // Is the left mouse button down?
            bool middle_button;      // Is the middle mouse button down?
            bool right_button;       // Is the right mouse button down?
            bool control;            // Is the CTRL key down?
            bool shift;              // Is the Shift key down?

            s32 x;                   // The X-position of the cursor relative to the upper-left corner of the client area.
            s32 y;                   // The Y-position of the cursor relative to the upper-left corner of the client area.
            s32 rel_x;               // How far the mouse moved since the last event.
            s32 rel_y;               // How far the mouse moved since the last event.
        };

        using mouse_enter_args = mouse_motion_args;
        using mouse_leave_args = args;

        class mouse_button_args : public args
        {
        public:
            enum class mouse_button
            {
                None = 0,
                Left = 1,
                Right = 2,
                Middle = 3
            };
            enum class button_state
            {
                Released = 0,
                Pressed = 1
            };

            mouse_button_args(mouse_button button_id, button_state state, bool left_button, bool middle_button, bool right_button, bool control, bool shift, s32 x, s32 y)
                :button(button_id)
                ,state(state)
                ,left_button(left_button)
                ,middle_button(middle_button)
                ,right_button(right_button)
                ,control(control)
                ,shift(shift)
                ,x(x)
                ,y(y)
            {}

            mouse_button button;      // The mouse button that was pressed or released.
            button_state state;       // Was the button pressed or released?
            bool left_button;        // Is the left mouse button down?
            bool middle_button;      // Is the middle mouse button down?
            bool right_button;       // Is the right mouse button down?
            bool control;            // Is the CTRL key down?
            bool shift;              // Is the Shift key down?

            s32 x;                   // The X-position of the cursor relative to the upper-left corner of the client area.
            s32 y;                   // The Y-position of the cursor relative to the upper-left corner of the client area.
        };

        class mouse_wheel_args : public args
        {
        public:
            mouse_wheel_args(float wheel_delta, bool left_button, bool middle_button, bool right_button, bool control, bool shift, s32 x, s32 y)
                :wheel_delta(wheel_delta)
                ,left_button(left_button)
                ,middle_button(middle_button)
                ,right_button(right_button)
                ,control(control)
                ,shift(shift)
                ,x(x)
                ,y(y)
            {}

            f32 wheel_delta;        // How much the mouse wheel has moved. A positive value indicates that the wheel was moved to the right. A negative value indicates the wheel was moved to the left.

            bool left_button;       // Is the left mouse button down?
            bool middle_button;     // Is the middle mouse button down?
            bool right_button;      // Is the right mouse button down?
            bool control;           // Is the CTRL key down?
            bool shift;             // Is the Shift key down?

            s32 x;                  // The X-position of the cursor relative to the upper-left corner of the client area.
            s32 y;                  // The Y-position of the cursor relative to the upper-left corner of the client area.
        };
    }
}