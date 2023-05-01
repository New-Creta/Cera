#include "entry.h"

#include <iostream>

#include <Windows.h>

namespace os
{
    application_creation_params app_entry(int argc, char** argv)
    {
        application_creation_params params;

        params.window_width = 1280;
        params.window_height = 720;
        params.window_title = "Sandbox";

        return params;
    }

    int app_initialize()
    {
        std::cout << "[CLIENT][INFO] Application Initialize" << std::endl;
        return S_OK;
    }

    int app_update()
    {
       return S_OK;
    }

    int app_render()
    {
        return S_OK;
    }

    int app_shutdown()
    {
        std::cout << "[CLIENT][INFO] Application Shutdown" << std::endl;
        return S_OK;
    }
}