#include "game_engine.h"

#include "abstract_game.h"

#include "generic_window.h"
#include "generic_window_definition.h"

#include "core_platform.h"
#include "core_globals.h"

#include "gui_application.h"

#include "util/log.h"

#include <string>
#include <memory>

namespace cera
{
    struct resolution_settings
    {
        s32 rex_x = 1280;
        s32 rex_y = 720;
        window_mode window_mode = window_mode::windowed;
    };

    resolution_settings g_system_resolution;

    game_engine::game_engine(gui_application& in_application, const std::shared_ptr<abstract_game>& in_game)
        :m_game_instance(in_game)
        ,m_application_instance(in_application)
    {}

    bool game_engine::initialize(s32 game_window_width, s32 game_window_height)
    {
        if(!m_game_instance->initialize())
        {
            log::error("Failed to initialize game");
            return false;
        }
        if(!m_game_instance->load_content())
        {
            log::error("Failed to load game content");
            return false;
        }

        m_game_window = create_game_window(game_window_width, game_window_height);
        m_game_window->set_window_mode(window_mode::windowed);
        m_game_window->show();

        return true;
    }
    
    void game_engine::tick()
    {
        m_update_clock.update();

        events::update_args update_args(m_update_clock.get_delta_seconds(), m_update_clock.get_total_seconds());
        m_game_instance->on_update(update_args);
    }

    void game_engine::shutdown()
    {
        m_game_instance->unload_content();
        m_game_instance->destroy();
        m_game_instance.reset();
    }

    std::shared_ptr<generic_window> game_engine::create_game_window(s32 game_window_width, s32 game_window_height)
    {
        s32 rex_x               = g_game_window_settings_override_enabled || game_window_width == -1 ? g_system_resolution.rex_x : game_window_width;
        s32 rex_y               = g_game_window_settings_override_enabled || game_window_height == -1 ? g_system_resolution.rex_y : game_window_height;
        window_mode window_mode = g_system_resolution.window_mode;
       
        // here we override settings given by the user on the command line
        // eg: -resx=1280 -resy=720
        //
        // conditionally_override_game_window_settings()

        std::optional<f32> win_x;
        std::optional<f32> win_y;

        // here we retrieve the window location from the user on the command line
        // eg: -winx=100, -winy=50
        //
        // win_x = parse_window_x_position();
        // win_y = parse_window_y_position();

        window_center_behaviour center_behaviour = window_center_behaviour::auto_center;
        if(win_x.has_value() || win_y.has_value())
        {
            center_behaviour = window_center_behaviour::none;
        }

	    auto get_project_setting_bool = [](const std::string& paramName, bool default) -> bool
	    	{
	    		bool temp = default;
	    		//GConfig->GetBool(TEXT("/Script/EngineSettings.GeneralProjectSettings"), *paramName, temp, GGameIni);
	    		return temp;
	    	};
	    auto get_project_setting_int = [](const std::string& paramName, s32 default) -> s32
	    {
	    	s32 temp = default;
	    	//GConfig->GetInt(TEXT("/Script/EngineSettings.GeneralProjectSettings"), *paramName, temp, GGameIni);
	    	return temp;
	    };
    
        const bool should_preserve_aspect_ratio = get_project_setting_bool("bShouldWindowPreserveAspectRatio", true);
	    const bool use_borderless_window = get_project_setting_bool("bUseBorderlessWindow", false) && PLATFORM_SUPPORTS_BORDERLESS_WINDOW;
	    const bool allow_window_resize = get_project_setting_bool("bAllowWindowResize", true);
	    const bool allow_close = get_project_setting_bool("bAllowClose", true);
	    const bool allow_maximize = get_project_setting_bool("bAllowMaximize", true);
	    const bool allow_minimize = get_project_setting_bool("bAllowMinimize", true);

	    const s32 min_window_width = get_project_setting_int("MinWindowWidth", 1280);
	    const s32 min_window_height = get_project_setting_int("MinWindowHeight", 720);

        generic_window_definition window_definition = generic_window_definition_builder()
            .set_type(window_type::normal)
            .set_x_position(win_x.has_value() ? win_x.value() : 0.0f)
            .set_y_position(win_y.has_value() ? win_y.value() : 0.0f)
            .set_width(rex_x)
            .set_height(rex_y)
            .set_has_os_window_border(!use_borderless_window)
            .set_appears_in_taskbar(true)
            .set_is_topmost_window(true)
            .set_accepts_input(true)
            .set_activation_policy(window_activation_policy::always)
            .set_center_behaviour(center_behaviour)
            .set_focus_when_first_shown(true)
            .set_has_close_button(allow_close)
            .set_supports_minimize(allow_minimize)
            .set_supports_maximize(allow_maximize)
            .set_is_modal_window(false)
            .set_is_regular_window(true)
            .set_has_sizing_frame(true)
            .set_preserve_aspect_ratio(should_preserve_aspect_ratio)
            .set_expected_max_width(-1)
            .set_expected_max_height(-1)
            .set_title(L"Cera")
            .set_opacity(1.0f)
            .set_corner_radius(2)
            .set_size_limits({})
            .build();

        const bool show_immediatly = true;

        return m_application_instance.add_window(window_definition, show_immediatly);
    }
}