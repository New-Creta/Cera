#include "application.h"
#include "gui.h"

#include "device/windows_declarations.h"
#include "device/windows_types.h"

#include "render/d3dx12_declarations.h"
#include "render/d3dx12_call.h"
#include "render/device.h"
#include "render/swapchain.h"
#include "render/command_list.h"
#include "render/command_queue.h"
#include "render/render_target.h"

#include "window.h"
#include "abstract_game.h"

#include "util/log.h"
#include "util/types.h"

namespace cera
{
    namespace adaptors
    {
        // A wrapper struct to allow shared pointers for the window class.
        // This is needed because the constructor and destructor for the Window
        // class are protected and not accessible by the std::make_shared method.
        struct make_window : public window
        {
            make_window(const window_desc& desc)
                : window(desc)
            {}

            ~make_window() override = default;
        };

        class make_swapchain : public swapchain
        {
        public:
            make_swapchain(device* device, window* window, DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM)
                : swapchain(*device, (HWND)window->get_window_handle(), backBufferFormat)
            {}

            ~make_swapchain() override = default;
        };

        class make_gui : public gui
        {
        public:
            make_gui(device* device, window* window, swapchain* swapchain)
                :gui(*device, window->get_window_handle(), swapchain->get_render_target())
            {}

            ~make_gui() override = default;
        };
    }

    std::unique_ptr<application> application::s_instance = nullptr;

    u64 application::s_frame_counter = 0;

    bool application::create(win::HInstance hinstance)
    {
        if (s_instance != nullptr)
        {
            log::warn("Application instance already exist, there can only be one application");
            return false;
        }

        s_instance = std::make_unique<application>(hinstance);

        return s_instance != nullptr;
    }

    void application::destroy()
    {
        s_instance.reset();
    }

    application* application::get()
    {
        return s_instance.get();
    }

    u64 application::get_frame_count()
    {
        return s_frame_counter;
    }

    application::application(win::HInstance hinstance)
        : m_hinstance(hinstance)
        , m_request_quit(false)
    {
        m_device = device::create(); 
    }

    application::~application()
    {
        atexit(&device::report_live_objects);
    }

    s32 application::run(abstract_game* game, s32 clientWidth, s32 clientHeight, const std::wstring& wndTitle)
    {
        if (!initialize(game, clientWidth, clientHeight, wndTitle))
        {
            return EXIT_FAILURE;
        }

        s32 result = loop(game);

        terminate(game);

        return result;
    }

    void application::quit()
    {
        // When called from another thread other than the main thread,
        // the WM_QUIT message goes to that thread and will not be handled
        // in the main thread. To circumvent this, we also set a boolean flag
        // to indicate that the user has requested to quit the application.
        m_request_quit = true;
    }

    const std::shared_ptr<device>& application::get_device() const
    {
        return m_device;
    }

    const std::shared_ptr<window>& application::get_window() const
    {
        return m_window;
    }

    const std::shared_ptr<swapchain>& application::get_swapchain() const
    {
        return m_swapchain;
    }

    const std::shared_ptr<gui> application::get_gui() const
    {
        return m_gui;
    }

    bool application::initialize(abstract_game* game, s32 clientWidth, s32 clientHeight, const std::wstring& wndTitle)
    {
        // Check for DirectX Math library support.
        if (!DirectX::XMVerifyCPUSupport())
        {
            log::error("Failed to verify DirectX Math library support.");
            return false;
        }

        window_callbacks callbacks;
        callbacks.update                = [game](const events::update_args& args)       { game->on_update(args); };
        callbacks.render                = [game](const events::render_args& args)       { game->on_render(args); };
        callbacks.render_gui            = [game](const events::render_gui_args& args)   { game->on_render_gui(args); };
        callbacks.resize                = [game](const events::resize_args& args)       { game->on_resize(args); };
        
        callbacks.key_pressed           = [game](const events::key_args& args)          { game->on_key_pressed(args); };
        callbacks.key_released          = [game](const events::key_args& args)          { game->on_key_released(args); };

        callbacks.mouse_motion          = [game](const events::mouse_motion_args& args) { game->on_mouse_moved(args); };
        callbacks.mouse_button_pressed  = [game](const events::mouse_button_args& args) { game->on_mouse_button_pressed(args); };
        callbacks.mouse_button_released = [game](const events::mouse_button_args& args) { game->on_mouse_button_released(args); };
        callbacks.mouse_wheel           = [game](const events::mouse_wheel_args& args)  { game->on_mouse_wheel(args); };

        window_desc wnd_desc;
        wnd_desc.hinstance = m_hinstance;
        wnd_desc.client_width = clientWidth;
        wnd_desc.client_height = clientHeight;
        wnd_desc.tearing_supported = m_device->is_tearing_supported();
        wnd_desc.window_class_name = L"DX12RenderWindowClass";
        wnd_desc.window_name = wndTitle.c_str();
        wnd_desc.window_proc = (win::WindowProcedureFunc)WndProc;
        wnd_desc.callbacks = callbacks;

        m_window = std::make_shared<adaptors::make_window>(wnd_desc);
        m_swapchain = std::make_shared<adaptors::make_swapchain>(m_device.get(), m_window.get(), DXGI_FORMAT_R8G8B8A8_UNORM);
        m_gui = std::make_shared<adaptors::make_gui>(m_device.get(), m_window.get(), m_swapchain.get());

        if (!game->initialize()) return false;
        if (!game->load_content()) return false;

        m_window->show();

        // Initialize the frame counter
        s_frame_counter = 0;

        return true;
    }

    void application::terminate(abstract_game* game)
    {
        // Flush any commands in the commands queues before quiting.
        m_device->flush();

        game->unload_content();
        game->destroy();

        m_gui.reset();
        m_swapchain.reset();
        m_device.reset();
    }

    s32 application::loop(abstract_game* game)
    {
        MSG msg = { 0 };
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                // Check to see of the application wants to quit.
                if (m_request_quit)
                {
                    ::PostQuitMessage(0);
                    m_request_quit = false;
                }
            }
        }

        return static_cast<s32>(msg.wParam);
    }
}

// Convert the message ID into a MouseButton ID
cera::events::mouse_button_args::mouse_button decode_mouse_button(UINT messageID)
{
    cera::events::mouse_button_args::mouse_button mouse_button = cera::events::mouse_button_args::mouse_button::None;
    switch (messageID)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    {
        mouse_button = cera::events::mouse_button_args::mouse_button::Left;
    }
    break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    {
        mouse_button = cera::events::mouse_button_args::mouse_button::Right;
    }
    break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    {
        mouse_button = cera::events::mouse_button_args::mouse_button::Middle;
    }
    break;
    }

    return mouse_button;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto gui = cera::application::get()->get_gui();
    
    // Allow for external handling of window messages.
    if (gui->wnd_proc_handler(hwnd, message, wParam, lParam))
    {
        return 1;
    }

    HWND win_hwnd = static_cast<HWND>(hwnd);
    if (message == WM_CREATE)
    {
        CREATESTRUCT* create_struct = reinterpret_cast<CREATESTRUCT*>(lParam);                                // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
        SetWindowLongPtr(win_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    }
    else
    {
        cera::window* wnd = reinterpret_cast<cera::window*>(GetWindowLongPtrW(win_hwnd, GWLP_USERDATA)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
        if (wnd)
        {
            switch (message)
            {
            case WM_PAINT:
            {
                ++cera::application::s_frame_counter;

                auto  device = cera::application::get()->get_device();
                auto  swapchain = cera::application::get()->get_swapchain();
                auto& command_queue = device->get_command_queue(D3D12_COMMAND_LIST_TYPE_DIRECT);
                auto  command_list = command_queue.get_command_list();

                // Update the window
                wnd->on_update();

                cera::application::get()->get_swapchain()->wait_for_swapchain();
                
                // Render everything to the screen
                wnd->on_render(command_list);

                // Render the GUI directly to the swap chain's render target.
                gui->new_frame();
                wnd->on_render_gui();
                gui->draw(command_list, swapchain->get_render_target());

                command_queue.execute_command_list(command_list);

                swapchain->present();
            }
            break;
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                MSG charMsg;
                // Get the Unicode character (UTF-16)
                unsigned int c = 0;
                // For printable characters, the next message will be WM_CHAR.
                // This message contains the character code we need to send the KeyPressed event.
                // Inspired by the SDL 1.2 implementation.
                if (PeekMessage(&charMsg, hwnd, 0, 0, PM_NOREMOVE) && charMsg.message == WM_CHAR)
                {
                    GetMessage(&charMsg, hwnd, 0, 0);
                    c = static_cast<unsigned int>(charMsg.wParam);
                }
                bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
                bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
                bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
                cera::key_code::Key key = (cera::key_code::Key)wParam;
                u32 scanCode = (lParam & 0x00FF0000) >> 16;
                cera::events::key_args key_args(key, c, cera::events::key_args::key_state::Pressed, shift, control, alt);
                wnd->on_key_pressed(key_args);
            }
            break;
            case WM_SYSKEYUP:
            case WM_KEYUP:
            {
                bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
                bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
                bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
                cera::key_code::Key key = (cera::key_code::Key)wParam;
                unsigned int c = 0;
                unsigned int scanCode = (lParam & 0x00FF0000) >> 16;

                // Determine which key was released by converting the key code and the scan code
                // to a printable character (if possible).
                // Inspired by the SDL 1.2 implementation.
                unsigned char keyboardState[256];
                GetKeyboardState(keyboardState);
                wchar_t translatedCharacters[4];
                if (int result = ToUnicodeEx(static_cast<UINT>(wParam), scanCode, keyboardState, translatedCharacters, 4, 0, NULL) > 0)
                {
                    c = translatedCharacters[0];
                }

                cera::events::key_args key_args(key, c, cera::events::key_args::key_state::Released, shift, control, alt);
                wnd->on_key_released(key_args);
            }
            break;
            // The default window procedure will play a system notification sound 
            // when pressing the Alt+Enter keyboard combination if this message is 
            // not handled.
            case WM_SYSCHAR:
                break;
            case WM_MOUSEMOVE:
            {
                bool l_button = (wParam & MK_LBUTTON) != 0;
                bool r_button = (wParam & MK_RBUTTON) != 0;
                bool m_button = (wParam & MK_MBUTTON) != 0;
                bool shift = (wParam & MK_SHIFT) != 0;
                bool control = (wParam & MK_CONTROL) != 0;

                int x = ((int)(short)LOWORD(lParam));
                int y = ((int)(short)HIWORD(lParam));

                cera::events::mouse_motion_args mouse_motion_args(l_button, m_button, r_button, control, shift, x, y);
                wnd->on_mouse_moved(mouse_motion_args);
            }
            break;
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
            {
                bool l_button = (wParam & MK_LBUTTON) != 0;
                bool r_button = (wParam & MK_RBUTTON) != 0;
                bool m_button = (wParam & MK_MBUTTON) != 0;
                bool shift = (wParam & MK_SHIFT) != 0;
                bool control = (wParam & MK_CONTROL) != 0;

                int x = ((int)(short)LOWORD(lParam));
                int y = ((int)(short)HIWORD(lParam));

                cera::events::mouse_button_args mouse_button_args(decode_mouse_button(message), cera::events::mouse_button_args::button_state::Pressed, l_button, m_button, r_button, control, shift, x, y);
                wnd->on_mouse_button_pressed(mouse_button_args);
            }
            break;
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP:
            {
                bool l_button = (wParam & MK_LBUTTON) != 0;
                bool r_button = (wParam & MK_RBUTTON) != 0;
                bool m_button = (wParam & MK_MBUTTON) != 0;
                bool shift = (wParam & MK_SHIFT) != 0;
                bool control = (wParam & MK_CONTROL) != 0;

                int x = ((int)(short)LOWORD(lParam));
                int y = ((int)(short)HIWORD(lParam));

                cera::events::mouse_button_args mouse_button_args(decode_mouse_button(message), cera::events::mouse_button_args::button_state::Released, l_button, m_button, r_button, control, shift, x, y);
                wnd->on_mouse_button_released(mouse_button_args);
            }
            break;
            case WM_MOUSELEAVE:
            {
                cera::events::mouse_leave_args mouse_leave_args;
                wnd->on_mouse_leave(mouse_leave_args);
            }
            break;
            case WM_MOUSEWHEEL:
            {
                // The distance the mouse wheel is rotated.
                // A positive value indicates the wheel was rotated to the right.
                // A negative value indicates the wheel was rotated to the left.
                float z_delta = ((int)(short)HIWORD(wParam)) / (float)WHEEL_DELTA;
                short key_states = (short)LOWORD(wParam);

                bool l_button = (wParam & MK_LBUTTON) != 0;
                bool r_button = (wParam & MK_RBUTTON) != 0;
                bool m_button = (wParam & MK_MBUTTON) != 0;
                bool shift = (key_states & MK_SHIFT) != 0;
                bool control = (key_states & MK_CONTROL) != 0;

                int x = ((int)(short)LOWORD(lParam));
                int y = ((int)(short)HIWORD(lParam));

                // Convert the screen coordinates to client coordinates.
                POINT client_to_screen_point;
                client_to_screen_point.x = x;
                client_to_screen_point.y = y;
                ScreenToClient(hwnd, &client_to_screen_point);

                cera::events::mouse_wheel_args mouse_wheel_args(z_delta, l_button, m_button, r_button, control, shift, (int)client_to_screen_point.x, (int)client_to_screen_point.y);
                wnd->on_mouse_wheel(mouse_wheel_args);
            }
            break;
            case WM_SIZE:
            {
                int width = ((int)(short)LOWORD(lParam));
                int height = ((int)(short)HIWORD(lParam));

                cera::application::get()->get_swapchain()->on_resize(width, height);

                cera::events::resize_args resize_args(width, height);
                wnd->on_resize(resize_args);
            }
            break;
            case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            break;

            default:
                return DefWindowProcW(hwnd, message, wParam, lParam);
            }
        }
        else
        {
            return DefWindowProcW(hwnd, message, wParam, lParam);
        }
    }

    return 0;
}