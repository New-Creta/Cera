#pragma once

#include "util/types.h"
#include "util/clock.h"

#include <memory>

namespace cera
{
    class abstract_game;
    class generic_window;
    class gui_application;

    class game_engine
    {
    public:
        game_engine(gui_application& application, const std::shared_ptr<abstract_game>& in_game);

        bool initialize(s32 game_window_width, s32 game_window_height);
        bool start();
        void tick();
        void end();
        void shutdown();

    private:
        std::shared_ptr<generic_window> create_game_window(s32 game_window_width, s32 game_window_height);

    private:
        std::shared_ptr<abstract_game> m_game_instance;
        std::shared_ptr<generic_window> m_game_window; 

        gui_application& m_application_instance;

        clock m_update_clock;
    };
}