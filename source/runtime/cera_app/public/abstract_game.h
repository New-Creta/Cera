#pragma once

#include "util/types.h"

#include "events.h"

namespace cera
{
    class abstract_game
    {
    public:
        virtual bool initialize() = 0;
        virtual bool load_content() = 0;

        virtual void on_update(const events::update_args& e)                        {/* nothing to implement */}
        virtual void on_render(const events::render_args& e)                        {/* nothing to implement */}
        virtual void on_render_gui(const events::render_gui_args& e)                {/* nothing to implement */}
        virtual void on_resize(const events::resize_args& e)                        {/* nothing to implement */}

        virtual void on_key_pressed(const events::key_args& e)                      {/* nothing to implement */ }
        virtual void on_key_released(const events::key_args& e)                     {/* nothing to implement */ }
        virtual void on_mouse_moved(const events::mouse_motion_args& e)             {/* nothing to implement */ }
        virtual void on_mouse_button_pressed(const events::mouse_button_args& e)    {/* nothing to implement */ }
        virtual void on_mouse_button_released(const events::mouse_button_args& e)   {/* nothing to implement */ }
        virtual void on_mouse_wheel(const events::mouse_wheel_args& e)              {/* nothing to implement */ }

        virtual void unload_content() = 0;
        virtual void destroy() = 0;
    };
}