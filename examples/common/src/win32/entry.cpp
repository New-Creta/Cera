#include "entry.h"

#include <Windows.h>
#include <shellapi.h>
#include <stdlib.h>

#include <iostream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // _CRT_SECURE_NO_WARNINGS
#endif

namespace os
{
    struct window_creation_params
    {
        window_viewport viewport;
        const char *title;
        int cmdshow;
        HINSTANCE hinstance;
    };

    struct window_context
    {
        HWND hwnd = NULL;
        HINSTANCE hinstance = NULL;
    };

    window_context s_ctx;

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        switch (message)
        {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            if(FAILED(os::app_render()))
            {
                std::cout << "[ERROR] Client code failed to render" << std::endl;

                EndPaint(hWnd, &ps);
                PostQuitMessage(1);

                return 0;
            }
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hWnd, message, wParam, lParam);
        }

        return 0;
    }

    class window
    {
    public:
        int create(const window_creation_params &params)
        {
            WNDCLASSEXA wnd_class;
            ZeroMemory(&wnd_class, sizeof(WNDCLASSEX));
            wnd_class.cbSize = sizeof(WNDCLASSEXA);
            wnd_class.style = CS_HREDRAW | CS_VREDRAW;
            wnd_class.lpfnWndProc = WndProc;
            wnd_class.cbClsExtra = 0;
            wnd_class.cbWndExtra = 0;
            wnd_class.hInstance = (HINSTANCE)params.hinstance;
            wnd_class.hIcon = NULL;
            wnd_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wnd_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wnd_class.lpszMenuName = nullptr;
            wnd_class.lpszClassName = params.title;
            wnd_class.hIconSm = NULL;

            if (!RegisterClassExA(&wnd_class))
            {
                std::cout << "[ERROR] Could not register window class" << std::endl;
                return E_FAIL;
            }

            s_ctx.hinstance = (HINSTANCE)params.hinstance;

            int x = params.viewport.x;
            int y = params.viewport.y;
            int width = params.viewport.width;
            int height = params.viewport.height;

            RECT rc = {0, 0, (LONG)width, (LONG)height};
            AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

            RECT desktop_rect;
            GetClientRect(GetDesktopWindow(), &desktop_rect);

            LONG screen_mid_x = (desktop_rect.right - desktop_rect.left) / 2;
            LONG screen_mid_y = (desktop_rect.bottom - desktop_rect.top) / 2;

            LONG half_x = (rc.right - rc.left) / 2;
            LONG half_y = (rc.bottom - rc.top) / 2;

            s_ctx.hwnd = CreateWindowA(params.title, params.title, WS_OVERLAPPEDWINDOW, x == 0 ? screen_mid_x - half_x : x, y == 0 ? screen_mid_y - half_y : y, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, (HINSTANCE)params.hinstance, this);

            if (!s_ctx.hwnd)
            {
                std::cout << "[ERROR] Invalid window handle was return by \"CreateWindowA\"" << std::endl;
                return E_FAIL;
            }

            ShowWindow(s_ctx.hwnd, params.cmdshow);
            SetForegroundWindow(s_ctx.hwnd);

            return S_OK;
        }

        void *get_primary_display_handle()
        {
            return s_ctx.hwnd;
        }

        void get_viewport(window_viewport &f)
        {
            RECT r;
            GetWindowRect(s_ctx.hwnd, &r);

            f.x = r.left;
            f.y = r.top;

            f.width = r.right - r.left;
            f.height = r.bottom - r.top;
        }

        void get_size(int &width, int &height)
        {
            RECT r;
            GetWindowRect(s_ctx.hwnd, &r);

            width = r.right - r.left;
            height = r.bottom - r.top;
        }

        float get_aspect()
        {
            int width;
            int height;

            get_size(width, height);

            return (float)width / (float)height;
        }
    };

    class application
    {
    public:
        application(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
            : m_hinstance(hInstance), m_hprevinstance(hPrevInstance), m_cmd_line(lpCmdLine), m_cmd_show(nCmdShow)
        {
        }

        int run(const application_creation_params &params)
        {
            int return_code = 0;

            if (FAILED(initialize(params)))
            {
                std::cout << "[ERROR] Application initialization failed." << std::endl;
                return E_FAIL;
            }
            if (FAILED(update()))
            {
                std::cout << "[ERROR] Application update failed." << std::endl;
                return E_FAIL;
            }
            if (FAILED(shutdown()))
            {
                std::cout << "[ERROR] Application shutdown failed." << std::endl;
                return E_FAIL;
            }

            return return_code;
        }

        void terminate(int returnCode /* = 0 */)
        {
            PostQuitMessage(returnCode);
        }

    private:
        int initialize(const application_creation_params &params)
        {
            window_creation_params wcp;
            wcp.cmdshow = m_cmd_show;
            wcp.hinstance = m_hinstance;
            wcp.title = params.window_title;
            wcp.viewport = {0, 0, params.window_width, params.window_height};

            if (FAILED(m_window.create(wcp)))
            {
                std::cout << "[ERROR] Failed to create window" << std::endl;
                return E_FAIL;
            }

            HWND hwnd = (HWND)m_window.get_primary_display_handle();

            if (FAILED(os::app_initialize()))
            {
                std::cout << "[ERROR] Client code failed to initialize" << std::endl;
                return E_FAIL;
            }

            return S_OK;
        }
        int update()
        {
            MSG msg = {0};
            while (msg.message != WM_QUIT)
            {
                if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }

                if (FAILED(os::app_update()))
                {                                                           
                    std::cout << "[ERROR] Client code failed to update" << std::endl;
                    return E_FAIL;
                }
            }

            return S_OK;
        }
        int shutdown()
        {
            if (FAILED(os::app_shutdown()))
            {
                std::cout << "[ERROR] Client code failed to shutdown" << std::endl;
                return E_FAIL;
            }

            return S_OK;
        }

    private:
        window m_window;

        HINSTANCE m_hinstance;
        HINSTANCE m_hprevinstance;
        LPTSTR m_cmd_line;
        int m_cmd_show;
    };

    //-------------------------------------------------------------------------
    struct command_line_arguments
    {
        command_line_arguments()
            : count(0), values(nullptr)
        {
        }
        command_line_arguments(const command_line_arguments &other)
            : count(0), values(nullptr)
        {
            if (other.count != 0)
            {
                count = other.count;
                values = new char *[other.count];
                std::memcpy(values, other.values, other.count);
            }
        }
        command_line_arguments(command_line_arguments &&other) noexcept
            : count(std::exchange(other.count, 0)), values(std::exchange(other.values, nullptr))
        {
        }
        ~command_line_arguments()
        {
            if (values != nullptr)
            {
                for (int i = 0; i < count; ++i)
                {
                    delete values[i];
                }

                delete[] values;
            }
        }

        command_line_arguments &operator=(const command_line_arguments &other)
        {
            if (other.count != 0)
            {
                count = other.count;
                values = new char *[other.count];
                std::memcpy(values, other.values, other.count);
            }
            else
            {
                count = 0;
                values = nullptr;
            }

            return *this;
        }
        command_line_arguments &operator=(command_line_arguments &&other) noexcept
        {
            count = std::exchange(other.count, 0);
            values = std::exchange(other.values, nullptr);

            return *this;
        }

        int count;
        char** values;
    };

    //-------------------------------------------------------------------------
    command_line_arguments get_command_line_arguments()
    {
        int argc;
        LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);

        command_line_arguments arguments;
        arguments.count = argc;
        arguments.values = new char *[argc];

        for (int i = 0; i < argc; ++i)
        {
            arguments.values[i] = new char[256];

            wcstombs(arguments.values[i], argv[i], 256);
        }

        LocalFree(argv);

        return arguments;
    }
}

//-------------------------------------------------------------------------
INT APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    os::command_line_arguments arguments = os::get_command_line_arguments();

    os::application_creation_params acp = os::app_entry(arguments.count, arguments.values);

    // console for std output..
    if (!AttachConsole(ATTACH_PARENT_PROCESS))
    {
        AllocConsole();
    }

    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    os::application app(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

    return app.run(acp);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif