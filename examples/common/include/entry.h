#pragma once

#include <Windows.h>

namespace os
{
    struct window_viewport
    {
        int x;
        int y;
        int width;
        int height;
    };

    struct application_creation_params
    {
        int         window_width        = 1280;
        int         window_height       = 720;
        const char* window_title        = "Application";
    };

    application_creation_params app_entry(int argc, char** argv);

    int app_initialize();
    int app_update();
    int app_render(HDC hdc, const PAINTSTRUCT& ps);
    int app_present();
    int app_shutdown();
}