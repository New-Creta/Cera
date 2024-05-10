#include "windows/win32_application_helpers.h"
#include "windows/win32_min.h"
#include "windows/win32_platform.h"
#include "windows/win32_application.h"
#include "windows/win32_system_includes.h"

#include "core_globals.h"

#include "util/guard_value.h"
#include "util/types.h"

#include <shcore.h>

#pragma comment(lib, "shcore.lib")

namespace cera
{
    namespace windows
    {
        namespace internal
        {
            static void win_pump_messages()
            {
                MSG msg;
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            void pump_messages_outside_main_loop()
            {
                const guard_value<bool> pump_message_guard(g_pumping_messages_outside_of_main_loop, true);

                // Process pending windows messages, which is necessary to the rendering thread in some cases where D3D
                // sends window messages (from IDXGISwapChain::Present) to the main thread owned viewport window.
                MSG msg;
                PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE | PM_QS_SENDMESSAGE);
            }
        }

        std::shared_ptr<generic_application> create_application()
        {
            return windows_application::create_windows_application( g_instance_handle, LoadIcon( (HINSTANCE)NULL, IDI_APPLICATION ) );
        }

        void pump_messages(bool from_main_loop)
        {
            const bool set_pumping_messages = !g_pumping_messages;

            if (set_pumping_messages)
            {
                g_pumping_messages = true;
            }

            if (!from_main_loop)
            {
                internal::pump_messages_outside_main_loop();
                
                if (set_pumping_messages)
                {
                    g_pumping_messages = false;
                }

                return;
            }

            g_pumping_messages_outside_of_main_loop = false;

            internal::win_pump_messages();

            if (set_pumping_messages)
            {
                g_pumping_messages = false;
            }
        }
    }
}