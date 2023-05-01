#include "entry.h"

#include "cera/swap_chain.h"
#include "cera/buffer.h"

#include <iostream>

#include <Windows.h>

namespace os
{
    cera::swap_chain& get_swapchain()
    {
        static cera::swap_chain s_swap_chain(1280, 720);

        return s_swap_chain;
    }

    void draw_something(cera::swap_chain& swapChain)
    {
        int wd = swapChain.width();
        int hgt = swapChain.height();

        static int x = 0;
        static int y = 0;
        static int x_vel = 4;
        static int y_vel = 7;

        if (x >= 0 && x < wd && y >= 0 && y < hgt)
        {
            swapChain.set_pixel(x, y, 0xffffffff);
        }

        x += x_vel;
        y += y_vel;
        if (x < 0 || x > wd)
        {
            x_vel *= -1;
        }
        if (y < 0 || y > hgt)
        {
            y_vel *= -1;
        }
    }

    application_creation_params app_entry(int argc, char **argv)
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

    int app_render(HDC hdc, const PAINTSTRUCT& ps)
    {
        HDC hdc_bmp = CreateCompatibleDC(hdc);
        auto old_bmp = SelectObject(hdc_bmp, get_swapchain().curr_buffer().get());

        BitBlt(hdc, 0, 0, get_swapchain().width(), get_swapchain().height(), hdc_bmp, 0, 0, SRCCOPY);

        SelectObject(hdc, old_bmp);
        DeleteDC(hdc_bmp);
        
        return S_OK;
    }

    int app_present()
    {
        draw_something(get_swapchain());

        get_swapchain().swap();
        get_swapchain().clear();

        return S_OK;
    }

    int app_shutdown()
    {
        std::cout << "[CLIENT][INFO] Application Shutdown" << std::endl;
        return S_OK;
    }
}