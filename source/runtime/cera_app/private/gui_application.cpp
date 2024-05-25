#include "gui_application.h"

#include "generic_application.h"

#include "core_platform.h"
#include "core_application.h"

namespace cera
{
    /** Application singleton */
    std::shared_ptr<gui_application> gui_application::s_current_application = nullptr;
    /** Holds a pointer to the platform application. */
	std::shared_ptr<generic_application> gui_application::s_platform_application = nullptr;

    gui_application::gui_application() = default;
    gui_application::~gui_application() = default;

    void gui_application::create()
    {
        create(std::shared_ptr<generic_application>(platform::create_application()));
    }

    std::shared_ptr<gui_application> gui_application::create(const std::shared_ptr<generic_application>& in_platform_application)
    {
        s_platform_application = in_platform_application;
        s_current_application = std::make_shared<gui_application>();

        s_platform_application->set_message_handler(s_current_application);

        return s_current_application;
    }

    /**
     * Returns true if a Slate application instance is currently initialized and ready
     *
     * @return  True if Slate application is initialized
     */
    bool gui_application::is_initialized()
    {
        return s_current_application != nullptr;
    }

    /**
     * Returns the current instance of the application. The application should have been initialized before
     * this method is called
     *
     * @return  Reference to the application
     */
    gui_application &gui_application::get()
    {
        assert(s_current_application != nullptr);
        return *s_current_application;
    }

    void gui_application::shutdown(bool shutdown_platform)
    {
        if (is_initialized())
        {
            s_current_application->on_shutdown();

            if (shutdown_platform)
            {
                s_platform_application->destroy_application();
            }

            s_platform_application.reset();
            s_current_application.reset();
        }
    }

    std::shared_ptr<generic_window> gui_application::add_window( const generic_window_definition& window_definition, const bool show)
    {
        std::shared_ptr<generic_window> window = s_platform_application->make_window();

        s_platform_application->initialize_window(window, window_definition, show);

        return window;
    }

    void gui_application::on_shutdown()
    {
        // Nothing to implement
    }
}