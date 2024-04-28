#include "demo.h"

#include "util/log.h"

namespace cera
{
    generic_application_creation_params entry()
    {
        generic_application_creation_params params;

        params.game = std::make_unique<demo>();
        params.game_window_width = 1600;
        params.game_window_height = 900;

        return params;
    }

    demo::demo()
    {
    }

    bool demo::initialize()
    {
        return true;
    }

    bool demo::load_content()
    {
        return true;
    }

    void demo::on_update(const events::update_args& e)
    {
        // Nothing to implement
    }

    void demo::on_render(const events::render_args& e)
    {
        // Nothing to implement
    }

    void demo::unload_content()
    {
        // Nothing to implement
    }

    void demo::destroy()
    {
        // Nothing to implement
    }
}