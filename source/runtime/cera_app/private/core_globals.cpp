#include "core_globals.h"

#ifndef CERA_CAN_EVER_RENDER
#define CERA_CAN_EVER_RENDER 1
#endif

namespace cera
{
    /** Whether the main loop is still running. */
    bool g_is_running = false;
    /** When enabled the user is able to specify command line arguments to modify the game window setup*/
    bool g_game_window_settings_override_enabled = false;
    /** Whether or not messages are being pumped outside of the main loop*/
    bool g_pumping_messages_outside_of_main_loop = false;
    /** Whether or not messages are being pumped */
    bool g_pumping_messages = false;
    /** Wheter the application can ever render. */
    bool g_can_ever_render = CERA_CAN_EVER_RENDER;
    /** Minimum width of a window */
    s32 g_minimum_window_width = 8;
    /** Maximum height of a window */
    s32 g_minimum_window_height = 8;
}