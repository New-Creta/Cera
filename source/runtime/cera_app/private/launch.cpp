#include "launch.h"

#include "gui_application.h"

#include "core_globals.h"
#include "core_platform.h"

#include "game_engine.h"

#include "generic_application_creation_params.h"

#include "util/log.h"

namespace cera
{
    namespace internal
    {
        std::unique_ptr<game_engine> g_engine = nullptr;
        
        static s32 engine_init(const std::shared_ptr<abstract_game>& game, s32 game_window_width, s32 game_window_height)
        {
            gui_application::create();

            g_engine = std::make_unique<game_engine>(gui_application::get(), game);
            if(!g_engine->initialize(game_window_width, game_window_height))
            {
                log::error("Game Engine initialization failed.");
                return 1;
            }

            g_is_running = true;
            return 0;
        }

        static void engine_run(const std::shared_ptr<abstract_game>& game)
        {
            constexpr bool from_main_loop = true;

            platform::pump_messages(from_main_loop);

            g_engine->tick();
        }

        static void engine_exit(const std::shared_ptr<abstract_game>& game)
        {
            g_is_running = false;

            g_engine->shutdown();
            g_engine.reset();
        }
    }

    s32 launch(const generic_application_creation_params& params)
    {
        // initialize the engine and close down on failure
        s32 error_level = internal::engine_init(params.game, params.game_window_width, params.game_window_height);
        
        if(error_level != 0)
        {
            internal::engine_exit(params.game);
            return error_level;
        }

        // run the engine
        while(!is_engine_exit_requested())
        {
            internal::engine_run(params.game);
        }

        internal::engine_exit(params.game);
        return 0;
    }
}