#include "core_globals.h"

#include "util/log.h"

namespace cera
{
    namespace internal
    {
        bool g_is_requesting_exit = false;	/* Indicates that MainLoop() should be exited at the end of the current iteration */
    }

    /** Whether the main loop is still running. */
    bool g_is_running = false;
    /** When enabled the user is able to specify command line arguments to modify the game window setup*/
    bool g_game_window_settings_override_enabled = false;
    /** Whether or not messages are being pumped outside of the main loop*/
    bool g_pumping_messages_outside_of_main_loop = false;
    /** Whether or not messages are being pumped */
    bool g_pumping_messages = false;

    /** Request that the engine exit as soon as it can safely do so
     * The "reason" is not optional and should be a useful description of why the engine exit is requested
     */
    void request_engine_exit(const std::string& reason)
    {
        log::info("Engine exit was requested with reason: {}", reason);
        
        internal::g_is_requesting_exit = true;
    }
    /** Check if an engine exit was requested */
    bool is_engine_exit_requested()
    {
        return internal::g_is_requesting_exit;
    }
}