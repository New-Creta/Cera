#include "launch.h"

#include "gui_application.h"

#include "core_application.h"
#include "core_globals.h"
#include "core_platform.h"

#include "game_engine.h"

#include "rhi.h"
#include "rhi_factory.h"

#include "generic_application_creation_params.h"

#include "util/log.h"

namespace cera
{
    using game_instance = std::shared_ptr<abstract_game>;

    class engine_loop
    {
      public:
        s32 init(const game_instance& game, s32 game_window_width, s32 game_window_height)
        {
            // Create the application
            gui_application::create();

            if (!init_rhi())
            {
                log::error("Renderer initialization failed.");
                return 1;
            }

            // Create the engine
            m_engine = std::make_unique<game_engine>(gui_application::get(), game);
            if (!m_engine->initialize(game_window_width, game_window_height))
            {
                log::error("Game Engine initialization failed.");
                return 1;
            }

            if (!m_engine->start())
            {
                log::error("Renderer post initialization failed.");
                return 1;
            }

            g_is_running = true;
            return 0;
        }

        void run(const game_instance& game)
        {
            constexpr bool from_main_loop = true;

            platform::pump_messages(from_main_loop);

            m_engine->tick();
        }

        void exit(const game_instance& game)
        {
            g_is_running = false;

            m_engine->end();
            m_engine->shutdown();
            m_engine.reset();
        }

      private:
        bool init_rhi()
        {
            m_rhi = renderer::rhi_factory::create();
            if (m_rhi == nullptr)
            {
                log::error("Unable to create RHI, engine will exit.");
                return false;
            }

#ifdef CERA_PLATFORM_WINDOWS
            // display_renderer_info();
#endif
            m_rhi->initialize();
            m_rhi->post_initialize();

            if (platform::is_engine_exit_requested())
            {
                log::error("Engine exit was requested");
                return false;
            }

            return true;
        }

      private:
        using game_engine_ptr = std::unique_ptr<game_engine>;
        using rhi_ptr = std::unique_ptr<renderer::rhi>;

        game_engine_ptr m_engine = nullptr;
        rhi_ptr m_rhi = nullptr;
    };

    s32 launch(const generic_application_creation_params& params)
    {
        engine_loop engine_loop;

        // initialize the engine and close down on failure
        s32 error_level = engine_loop.init(params.game, params.game_window_width, params.game_window_height);

        if (error_level != 0)
        {
            engine_loop.exit(params.game);
            return error_level;
        }

        // run the engine
        while (!platform::is_engine_exit_requested())
        {
            engine_loop.run(params.game);
        }

        engine_loop.exit(params.game);
        return 0;
    }
} // namespace cera