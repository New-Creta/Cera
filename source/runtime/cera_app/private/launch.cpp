#include "launch.h"

#include "gui_application.h"

#include "core_globals.h"
#include "core_platform.h"

#include "game_engine.h"

#include "rhi.h"
#include "rhi_factory.h"

#include "generic_application_creation_params.h"

#include "util/log.h"

namespace cera
{
    namespace internal
    {
        std::unique_ptr<game_engine> g_engine = nullptr;
        std::unique_ptr<renderer::rhi> g_rhi = nullptr;

        static bool init_null_rhi()
        {
            if (renderer::rhi_factory::is_supported())
            {
                g_rhi = renderer::rhi_factory::create();

#ifdef CERA_PLATFORM_WINDOWS
                // display_renderer_info();
#endif
                g_rhi->initialize();
                g_rhi->post_initialize();
            }
        }

        static bool init_platform_rhi()
        {
            if (renderer::rhi_factory::is_supported())
            {
                g_rhi = renderer::rhi_factory::create();

#ifdef CERA_PLATFORM_WINDOWS
                // display_renderer_info();
#endif
                g_rhi->initialize();
                g_rhi->post_initialize();
            }
        }
        
        static s32 engine_init(const std::shared_ptr<abstract_game>& game, s32 game_window_width, s32 game_window_height)
        {
            // Create the application
            gui_application::create();

            if (!g_can_ever_render)
            {
                if (!init_null_rhi())
                {
                    log::error("Renderer initialization failed.");
                    return false;
                }
            }
            else
            {
                if (!init_platform_rhi())
                {
                    log::error("Renderer initialization failed.");
                    return false;
                }
            }

            // Create the engine
            g_engine = std::make_unique<game_engine>(gui_application::get(), game);
            if(!g_engine->initialize(game_window_width, game_window_height))
            {
                log::error("Game Engine initialization failed.");
                return 1;
            }

            // // Post initialize the renderer
            // if (renderer::post_initialize() == false) // NOLINT(readability-simplify-boolean-expr)
            // {
            //     log::error("Renderer post initialization failed.");
            //     return 1;
            // }
            //
            // renderer::flush();

            // Post initialize the engine
            if (!g_engine->start())
            {
                log::error("Renderer post initialization failed.");
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

            g_engine->end();
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