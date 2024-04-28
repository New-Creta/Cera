#pragma once

#include <string>

namespace cera
{
    /** Whether the main loop is still running. */
    extern bool g_is_running;

    /** When enabled the user is able to specify command line arguments to modify the game window setup*/
    extern bool g_game_window_settings_override_enabled;

    /** Whether or not messages are being pumped outside of main loop */
    extern bool g_pumping_messages_outside_of_main_loop;

    /** Whether or not messages are being pumped */
    extern bool g_pumping_messages;

    /** Request that the engine exit as soon as it can safely do so
     * The "reason" is not optional and should be a useful description of why the engine exit is requested
     */
    void request_engine_exit(const std::string& reason);
    /** Check if an engine exit was requested */
    bool is_engine_exit_requested();
}