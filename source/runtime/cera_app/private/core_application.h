#pragma once

#include "core_platform.h"

#ifdef CERA_PLATFORM_WINDOWS

// Platform includes
#include "windows/win32_application_helpers.h"

namespace cera
{   
    namespace windows
    {
        extern bool g_platform_supports_borderless_window;
    }
}

#endif