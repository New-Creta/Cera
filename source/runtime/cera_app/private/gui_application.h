#pragma once

#include "generic_application_message_handler.h"
#include "generic_window.h"

#include <assert.h>

namespace cera
{
    class generic_application;

    class gui_application : public generic_application_message_handler
    {
    public:
        gui_application();
        ~gui_application() override;

    public:
        static void create();
        static std::shared_ptr<gui_application> create(const std::shared_ptr<generic_application> &in_platform_application);

        /**
         * Returns true if a GUI application instance is currently initialized and ready
         *
         * @return  True if GUI application is initialized
         */
        static bool is_initialized();

        /**
         * Returns the current instance of the application. The application should have been initialized before
         * this method is called
         *
         * @return  Reference to the application
         */
        static gui_application& get();

        /** Shutdown the application */
        static void shutdown(bool shutdown_platform_app = true);

    public:
        /** Add a new window to the application */
        static std::shared_ptr<generic_window> add_window( const generic_window_definition& window_definition, const bool show = true );

    public:
        void initialize_renderer();

    protected:
        /** Event when application is closing down */
        void on_shutdown();

    private:
        /** Application singleton */
        static std::shared_ptr<gui_application> s_current_application;
        /** Holds a pointer to the platform application. */
	    static std::shared_ptr<generic_application> s_platform_application;
    };
}