#include "windows/win32_application.h"
#include "windows/win32_window.h"

#include "generic_application_message_handler.h"

#include "core_globals.h"

#include "util/log.h"

#include <assert.h>

namespace cera
{
    namespace internal
    {
        static std::shared_ptr<windows_window> find_window_by_hwnd(const std::vector<std::shared_ptr<windows_window>> &windows_to_search, HWND handle_to_find)
        {
            for (s32 window_index = 0; window_index < windows_to_search.size(); ++window_index)
            {
                std::shared_ptr<windows_window> window = windows_to_search[window_index];
                if (window->get_hwnd() == handle_to_find)
                {
                    return window;
                }
            }

            return std::shared_ptr<windows_window>(nullptr);
        }
    }

    std::shared_ptr<windows_application> g_windows_application = nullptr;

    const POINT windows_application::s_minimized_window_position = { -32000, -32000 };

    bool windows_application::register_class(const HINSTANCE hinstance, const HICON hicon)
    {
        WNDCLASS wc;
        memset(&wc, 0, sizeof(WNDCLASS));
        wc.style = CS_DBLCLKS; // We want to receive double clicks
        wc.lpfnWndProc = app_wnd_proc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hinstance;
        wc.hIcon = hicon;
        wc.hCursor = NULL;       // We manage the cursor ourselves
        wc.hbrBackground = NULL; // Transparent
        wc.lpszMenuName = NULL;
        wc.lpszClassName = windows_window::s_app_window_class;

        if (!::RegisterClass(&wc))
        {
            MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
            return false;
        }

        return true;
    }

    bool windows_application::is_keyboard_input_message(u32 msg)
    {
        switch (msg)
        {
        // Keyboard input notification messages...
        case WM_CHAR:
        case WM_SYSCHAR:
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYUP:
        case WM_SYSCOMMAND:
            return true;
        }
        return false;
    }

    bool windows_application::is_mouse_input_message(u32 msg)
    {
        switch (msg)
        {
        // Mouse input notification messages...
        case WM_MOUSEHWHEEL:
        case WM_MOUSEWHEEL:
        case WM_MOUSEHOVER:
        case WM_MOUSELEAVE:
        case WM_MOUSEMOVE:
        case WM_NCMOUSEHOVER:
        case WM_NCMOUSELEAVE:
        case WM_NCMOUSEMOVE:
        case WM_NCMBUTTONDBLCLK:
        case WM_NCMBUTTONDOWN:
        case WM_NCMBUTTONUP:
        case WM_NCRBUTTONDBLCLK:
        case WM_NCRBUTTONDOWN:
        case WM_NCRBUTTONUP:
        case WM_NCXBUTTONDBLCLK:
        case WM_NCXBUTTONDOWN:
        case WM_NCXBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_XBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
            return true;
        }
        return false;
    }

    /**  @return  True if a windows message is related to user input (mouse, keyboard) */
    bool windows_application::is_input_message(u32 msg)
    {
        if (is_keyboard_input_message(msg) || is_mouse_input_message(msg))
        {
            return true;
        }

        switch (msg)
        {
        // Raw input notification messages...
        case WM_INPUT:
        case WM_INPUT_DEVICE_CHANGE:
            return true;
        }
        return false;
    }

    // Defined as a global so that it can be extern'd by UELibrary
    LRESULT windows_application_wnd_proc(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam)
    {
        return g_windows_application->process_message(hwnd, msg, wparam, lparam);
    }

    LRESULT CALLBACK windows_application::app_wnd_proc(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam)
    {
        return windows_application_wnd_proc(hwnd, msg, wparam, lparam);
    }

    s32 windows_application::process_message(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam)
    {
        std::shared_ptr<windows_window> current_native_event_window = internal::find_window_by_hwnd(m_windows, hwnd);

        if (m_windows.size() && current_native_event_window)
        {
            bool message_externally_handled = false;
            s32 external_message_handler_result = 0;

            // give others a chance to handle messages
            for (auto &handler : m_message_handlers)
            {
                s32 handler_result = 0;
                if (handler->process_message(hwnd, msg, wparam, lparam, handler_result))
                {
                    if (!message_externally_handled)
                    {
                        message_externally_handled = true;
                        external_message_handler_result = handler_result;
                    }
                }
            }

            switch (msg)
            {
            case WM_CHAR:
            {
                defer_message(current_native_event_window, hwnd, msg, wparam, lparam);
                return 0;
            }
            break;
            case WM_SYSCHAR:
            {
                if (!m_consome_alt_space && (HIWORD(lparam) & 0x2000) != 0 && wparam == VK_SPACE)
                {
                    // Do not handle Alt+Space so that it passes through and opens the window system menu
                    break;
                }
                else
                {
                    return 0;
                }
            }
            break;
            case WM_SYSKEYDOWN:
            {
                // Alt-F4 or Alt+Space was pressed.
                if (wparam == VK_F4)
                {
                    // Allow alt+f4 to close the window, but write a log warning
                    log::info("Alt-F4 pressed!");
                }
                // If we're consuming alt+space, pass it along
                else if (m_consome_alt_space || wparam != VK_SPACE)
                {
                    defer_message(current_native_event_window, hwnd, msg, wparam, lparam);
                }
            }
            break;
            case WM_KEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYUP:
            case WM_LBUTTONDBLCLK:
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_XBUTTONDBLCLK:
            case WM_XBUTTONDOWN:
            case WM_XBUTTONUP:
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
            case WM_NCMOUSEMOVE:
            case WM_MOUSEMOVE:
            case WM_MOUSEWHEEL:
            {
                defer_message(current_native_event_window, hwnd, msg, wparam, lparam);
                return 0;
            }
            break;
            case WM_SHOWWINDOW:
            {
                defer_message(current_native_event_window, hwnd, msg, wparam, lparam);
            }
            break;
            case WM_SIZE:
            {
                defer_message(current_native_event_window, hwnd, msg, wparam, lparam);

                const bool was_maximized = (wparam == SIZE_MAXIMIZED);
                const bool was_restored = (wparam == SIZE_RESTORED);

                if (was_maximized || was_restored)
                {
                    get_message_handler()->on_window_action(current_native_event_window, was_maximized ? window_action::maximize : window_action::restore);
                }

                return 0;
            }
            break;
            case WM_SIZING:
            {
                defer_message(current_native_event_window, hwnd, msg, wparam, lparam, 0, 0);

                if (current_native_event_window->get_definition().should_preserve_aspect_ratio)
                {
                    // The rect we get in lparam is window rect, but we need to preserve client's aspect ratio,
                    // so we need to find what the border and title bar sizes are, if window has them and adjust the rect.
                    WINDOWINFO window_info;
                    memset(&window_info, 0, sizeof(WINDOWINFO));
                    window_info.cbSize = sizeof(window_info);
                    ::GetWindowInfo(hwnd, &window_info);

                    RECT test_rect;
                    test_rect.left = test_rect.right = test_rect.top = test_rect.bottom = 0;
                    AdjustWindowRectEx(&test_rect, window_info.dwStyle, false, window_info.dwExStyle);

                    RECT *new_rect = (RECT *)lparam;
                    new_rect->left -= test_rect.left;
                    new_rect->right -= test_rect.right;
                    new_rect->top -= test_rect.top;
                    new_rect->bottom -= test_rect.bottom;

                    const f32 aspect_ratio = current_native_event_window->get_aspect_ratio();
                    s32 new_width = new_rect->right - new_rect->left;
                    s32 new_height = new_rect->bottom - new_rect->top;

                    window_size_limits size_limits = current_native_event_window->get_definition().size_limits;

                    switch (wparam)
                    {
                    case WMSZ_LEFT:
                    case WMSZ_RIGHT:
                    case WMSZ_BOTTOMLEFT:
                    case WMSZ_BOTTOMRIGHT:
                    case WMSZ_TOPLEFT:
                    case WMSZ_TOPRIGHT:
                    {
                        s32 min_width = (s32)size_limits.get_min_width().value();
                        if (size_limits.get_min_height().value() < size_limits.get_min_width().value())
                        {
                            min_width = (s32)(size_limits.get_min_height().value() * aspect_ratio);
                        }

                        if (new_width < min_width)
                        {
                            if (wparam == WMSZ_LEFT || wparam == WMSZ_BOTTOMLEFT || wparam == WMSZ_TOPLEFT)
                            {
                                new_rect->left -= (min_width - new_width);
                            }
                            else if (wparam == WMSZ_RIGHT || wparam == WMSZ_BOTTOMRIGHT || wparam == WMSZ_TOPRIGHT)
                            {
                                new_rect->right += (min_width - new_width);
                            }

                            new_width = min_width;
                        }

                        break;
                    }
                    case WMSZ_TOP:
                    case WMSZ_BOTTOM:
                    {
                        s32 min_height = (s32)size_limits.get_min_height().value();
                        if (size_limits.get_min_width().value() < size_limits.get_min_height().value())
                        {
                            min_height = (s32)(size_limits.get_min_width().value() / aspect_ratio);
                        }

                        if (new_height < min_height)
                        {
                            if (wparam == WMSZ_TOP)
                            {
                                new_rect->top -= (min_height - new_height);
                            }
                            else
                            {
                                new_rect->bottom += (min_height - new_height);
                            }

                            new_height = min_height;
                        }
                        break;
                    }
                    }

                    switch (wparam)
                    {
                    case WMSZ_LEFT:
                    case WMSZ_RIGHT:
                    {
                        s32 adjusted_height = (s32)((f32)new_width / aspect_ratio);
                        new_rect->top -= (adjusted_height - new_height) / 2;
                        new_rect->bottom += (adjusted_height - new_height) / 2;
                        break;
                    }
                    case WMSZ_TOP:
                    case WMSZ_BOTTOM:
                    {
                        s32 adjusted_width = (s32)((f32)new_height * aspect_ratio);
                        new_rect->left -= (adjusted_width - new_width) / 2;
                        new_rect->right += (adjusted_width - new_width) / 2;
                        break;
                    }
                    case WMSZ_TOPLEFT:
                    {
                        s32 adjusted_height = (s32)((f32)new_width / aspect_ratio);
                        new_rect->top -= adjusted_height - new_height;
                        break;
                    }
                    case WMSZ_TOPRIGHT:
                    {
                        s32 adjusted_height = (s32)((f32)new_width / aspect_ratio);
                        new_rect->top -= adjusted_height - new_height;
                        break;
                    }
                    case WMSZ_BOTTOMLEFT:
                    {
                        s32 adjusted_height = (s32)((f32)new_width / aspect_ratio);
                        new_rect->bottom += adjusted_height - new_height;
                        break;
                    }
                    case WMSZ_BOTTOMRIGHT:
                    {
                        s32 adjusted_height = (s32)((f32)new_width / aspect_ratio);
                        new_rect->bottom += adjusted_height - new_height;
                        break;
                    }
                    }

                    AdjustWindowRectEx(new_rect, window_info.dwStyle, false, window_info.dwExStyle);

                    return TRUE;
                }
            }
            break;
            case WM_ENTERSIZEMOVE:
            {
                m_in_modal_size_loop = true;
                defer_message(current_native_event_window, hwnd, msg, wparam, lparam, 0, 0);
            }
            break;
            case WM_EXITSIZEMOVE:
            {
                m_in_modal_size_loop = false;
                defer_message(current_native_event_window, hwnd, msg, wparam, lparam, 0, 0);
            }
            break;
            case WM_MOVE:
            {
                // client area position
                const s32 new_x = (s32)(s16)(LOWORD(lparam));
                const s32 new_y = (s32)(s16)(HIWORD(lparam));
                const POINT new_position = {new_x, new_y};

                // Only cache the screen position if its not minimized
                if (windows_application::s_minimized_window_position.x != new_position.x || windows_application::s_minimized_window_position.y != new_position.y)
                {
                    get_message_handler()->on_moved_window(current_native_event_window, new_x, new_y);

                    return 0;
                }
            }
            break;
            case WM_MOUSEACTIVATE:
            {
                defer_message(current_native_event_window, hwnd, msg, wparam, lparam);
            }
            break;
            case WM_ACTIVATE:
            {
                defer_message(current_native_event_window, hwnd, msg, wparam, lparam);
            }
            break;
            case WM_ACTIVATEAPP:
            {
                // When window activation changes we are not in a modal size loop
                m_in_modal_size_loop = false;
                defer_message(current_native_event_window, hwnd, msg, wparam, lparam);
            }
            break;
            case WM_PAINT:
            {
                if (m_in_modal_size_loop)
                {
                    get_message_handler()->on_os_paint(current_native_event_window);
                }
            }
            break;
            case WM_ERASEBKGND:
            {
                // Intercept background erasing to eliminate flicker.
                // Return non-zero to indicate that we'll handle the erasing ourselves
                return 1;
            }
            break;
            case WM_NCACTIVATE:
            {
                if (!current_native_event_window->get_definition().has_os_window_border)
                {
                    // Unless using the OS window border, intercept calls to prevent non-client area drawing a border upon activation or deactivation
                    // Return true to ensure standard activation happens
                    return true;
                }
            }
            break;
            case WM_NCPAINT:
            {
                if (!current_native_event_window->get_definition().has_os_window_border)
                {
                    // Unless using the OS window border, intercept calls to draw the non-client area - we do this ourselves
                    return 0;
                }
            }
            break;
            case WM_DESTROY:
            {
                auto it = std::find(m_windows.begin(), m_windows.end(), current_native_event_window);
                if (it != m_windows.end())
                {
                    m_windows.erase(it);
                }
                return 0;
            }
            break;
            case WM_CLOSE:
            {
                defer_message(current_native_event_window, hwnd, msg, wparam, lparam);
                return 0;
            }
            break;
            case WM_SYSCOMMAND:
            {
                switch (wparam & 0xfff0)
                {
                case SC_RESTORE:
                    // Checks to see if the window is minimized.
                    if (IsIconic(hwnd))
                    {
                        // This is required for restoring a minimized fullscreen window
                        ::ShowWindow(hwnd, SW_RESTORE);
                        return 0;
                    }
                    else
                    {
                        if (!get_message_handler()->on_window_action(current_native_event_window, window_action::restore))
                        {
                            return 1;
                        }
                    }
                    break;
                case SC_MAXIMIZE:
                {
                    if (!get_message_handler()->on_window_action(current_native_event_window, window_action::maximize))
                    {
                        return 1;
                    }
                }
                break;
                case SC_CLOSE:
                {
                    defer_message(current_native_event_window, hwnd, WM_CLOSE, 0, 0);
                    return 1;
                }
                break;
                default:
                    if (!is_input_message(msg))
                    {
                        return 0;
                    }
                    break;
                }
            }
            break;
            case WM_GETMINMAXINFO:
            {
                MINMAXINFO *min_max_info = (MINMAXINFO *)lparam;
                window_size_limits size_limits = current_native_event_window->get_definition().size_limits;

                // We need to inflate the max values if using an OS window border
                s32 border_width = 0;
                s32 border_height = 0;
                if (current_native_event_window->get_definition().has_os_window_border)
                {
                    const DWORD window_style = ::GetWindowLong(hwnd, GWL_STYLE);
                    const DWORD window_ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);

                    // This adjusts a zero rect to give us the size of the border
                    RECT border_rect = {0, 0, 0, 0};
                    ::AdjustWindowRectEx(&border_rect, window_style, false, window_ex_style);

                    border_width = border_rect.right - border_rect.left;
                    border_height = border_rect.bottom - border_rect.top;
                }

                // We always apply border_width and BorderHeight since Slate always works with client area window sizes
                min_max_info->ptMinTrackSize.x = static_cast<s32>(std::round(size_limits.get_min_width().value_or((f32)min_max_info->ptMinTrackSize.x)));
                min_max_info->ptMinTrackSize.y = static_cast<s32>(std::round(size_limits.get_min_height().value_or((f32)min_max_info->ptMinTrackSize.y)));
                min_max_info->ptMaxTrackSize.x = static_cast<s32>(std::round(size_limits.get_max_width().value_or((f32)min_max_info->ptMaxTrackSize.x)) + border_width);
                min_max_info->ptMaxTrackSize.y = static_cast<s32>(std::round(size_limits.get_max_height().value_or((f32)min_max_info->ptMaxTrackSize.y)) + border_height);
                return 0;
            }
            break;
            case WM_NCLBUTTONDOWN:
            case WM_NCRBUTTONDOWN:
            case WM_NCMBUTTONDOWN:
            {
                switch (wparam)
                {
                case HTMINBUTTON:
                {
                    if (!get_message_handler()->on_window_action(current_native_event_window, window_action::clicked_non_client_area))
                    {
                        return 1;
                    }
                }
                break;
                case HTMAXBUTTON:
                {
                    if (!get_message_handler()->on_window_action(current_native_event_window, window_action::clicked_non_client_area))
                    {
                        return 1;
                    }
                }
                break;
                case HTCLOSE:
                {
                    if (!get_message_handler()->on_window_action(current_native_event_window, window_action::clicked_non_client_area))
                    {
                        return 1;
                    }
                }
                break;
                case HTCAPTION:
                {
                    if (!get_message_handler()->on_window_action(current_native_event_window, window_action::clicked_non_client_area))
                    {
                        return 1;
                    }
                }
                break;
                }
            }
            break;
            case WM_GETDLGCODE:
            {
                // Slate wants all keys and messages.
                return DLGC_WANTALLKEYS;
            }
            break;
            case WM_CREATE:
            {
                return 0;
            }
            break;
            default:
            {
                if (message_externally_handled)
                {
                    return external_message_handler_result;
                }
            }
            break;
            }
        }

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    std::shared_ptr<windows_application> windows_application::create_windows_application(const HINSTANCE hinstance, const HICON hicon)
    {
        g_windows_application = std::make_shared<windows_application>(hinstance, hicon);

        return g_windows_application;
    }

    windows_application::windows_application(const HINSTANCE hinstance, const HICON hicon)
        : generic_application()
        , m_instance_handle(hinstance)
        , m_consome_alt_space(false)
        , m_in_modal_size_loop(false)
        , m_allowed_to_defer_message_processing(true)
        , m_force_activate_by_mouse(false)
    {
        memset(m_modifier_key_state, 0, modifier_key::count);

        // Disable the process from being showing "ghosted" not responding messages during slow tasks
        // This is a hack.  A more permanent solution is to make our slow tasks not block the editor for so long
        // that message pumping doesn't occur (which causes these messages).
        ::DisableProcessWindowsGhosting();

        // Register the Win32 class for Slate windows and assign the application instance and icon
        if (!register_class(hinstance, hicon))
        {
            log::error("Unable to execute RegisterClass");
            return;
        }
    }

    windows_application::~windows_application() = default;

    void windows_application::process_deferred_events() 
    {
        // This function can be reentered when entering a modal tick loop.
        // We need to make a copy of the events that need to be processed or we may end up processing the same messages twice 
        std::vector<deferred_windows_message> events_to_process( m_deferred_messages );

        m_deferred_messages.clear();
        for( s32 message_index = 0; message_index < events_to_process.size(); ++message_index )
        {
            const deferred_windows_message& deferred_message = events_to_process[message_index];
            process_deferred_message( deferred_message );
        }

        check_for_shift_up_events(VK_LSHIFT);
        check_for_shift_up_events(VK_RSHIFT);
    }

    std::shared_ptr<generic_window> windows_application::make_window()
    {
        return windows_window::make();
    }

    void windows_application::initialize_window(const std::shared_ptr<generic_window> &in_window, const generic_window_definition& definition, const bool show)
    {
        const std::shared_ptr<windows_window> window = std::static_pointer_cast<windows_window>(in_window);

        m_windows.push_back(window);

        window->initialize(this, definition, m_instance_handle, show);
    }

    void windows_application::destroy_application()
    {
        g_windows_application.reset();
    }

    void windows_application::add_message_handler(const std::shared_ptr<iwindows_message_handler> &in_message_handler)
    {
        m_message_handlers.push_back(in_message_handler);
    }

    void windows_application::remove_message_handler(const std::shared_ptr<iwindows_message_handler> &in_message_handler)
    {
        auto it = std::find(m_message_handlers.begin(), m_message_handlers.end(), in_message_handler);
        if (it != m_message_handlers.end())
        {
            m_message_handlers.erase(it);
        }
    }

    /** Defers a m_windows message for later processing. */
    void windows_application::defer_message(std::shared_ptr<class windows_window> &native_window, HWND in_hwnd, u32 in_message, WPARAM in_wparam, LPARAM in_lparam, s32 mouse_x, s32 mouse_y, u32 raw_input_flags)
    {
        if (g_pumping_messages_outside_of_main_loop && m_allowed_to_defer_message_processing)
        {
            m_deferred_messages.push_back(deferred_windows_message(native_window, in_hwnd, in_message, in_wparam, in_lparam, mouse_x, mouse_y, raw_input_flags));
        }
        else
        {
            // When not deferring messages, process them immediately
            process_deferred_message(deferred_windows_message(native_window, in_hwnd, in_message, in_wparam, in_lparam, mouse_x, mouse_y, raw_input_flags));
        }
    }

    void windows_application::check_for_shift_up_events(const s32 key_code)
    {
        assert(key_code == VK_LSHIFT || key_code == VK_RSHIFT);

        // Since VK_SHIFT doesn't get an up message if the other shift key is held we need to poll for it
        const modifier_key modifier_key_index = key_code == VK_LSHIFT ? modifier_key::left_shift : modifier_key::right_shift;
        if (m_modifier_key_state[modifier_key_index] && ((::GetKeyState(key_code) & 0x8000) == 0) )
        {
            m_modifier_key_state[modifier_key_index] = false;
            get_message_handler()->on_key_up( key_code, 0, false );
        }
    }

    void windows_application::update_all_modifier_key_states()
    {
        m_modifier_key_state[modifier_key::left_shift]		= (::GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;
        m_modifier_key_state[modifier_key::right_shift]		= (::GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0;
        m_modifier_key_state[modifier_key::left_control]	= (::GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
        m_modifier_key_state[modifier_key::right_control]	= (::GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0;
        m_modifier_key_state[modifier_key::left_alt]		= (::GetAsyncKeyState(VK_LMENU) & 0x8000) != 0;
        m_modifier_key_state[modifier_key::right_alt]		= (::GetAsyncKeyState(VK_RMENU) & 0x8000) != 0;
        m_modifier_key_state[modifier_key::caps_lock]		= (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
    }

    s32 windows_application::process_deferred_message(const deferred_windows_message& deferred_message)
    {
        if (!m_windows.size() && deferred_message.native_window.use_count())
        {
            HWND hwnd = deferred_message.hwnd;
            u32 msg = deferred_message.message;
            WPARAM wparam = deferred_message.wparam;
            LPARAM lparam = deferred_message.lparam;

            std::shared_ptr<windows_window> current_native_event_window = deferred_message.native_window.lock();

            // This effectively disables a window without actually disabling it natively with the OS.
            // This allows us to continue receiving messages for it.
            if (!is_input_message(msg))
            {
                if (is_keyboard_input_message(msg))
                {
                    // Force an update since we may have just consumed a modifier key state change
                    update_all_modifier_key_states();
                }
                return 0; // consume input messages
            }

            switch (msg)
            {
                // character
            case WM_CHAR:
            {
                // character code is stored in WPARAM
                const tchar character = static_cast<tchar>(wparam);

                // LPARAM bit 30 will be ZERO for new presses, or ONE if this is a repeat
                const bool is_repeat = (lparam & 0x40000000) != 0;

                get_message_handler()->on_key_char(character, is_repeat);

                // Note: always return 0 to handle the message.  Win32 beeps if WM_CHAR is not handled...
                return 0;
            }
            break;

                // Key down
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                // character code is stored in WPARAM
                const s32 win32_key = static_cast<s32>(wparam);

                // The actual key to use.  Some keys will be translated into other keys.
                // I.E VK_CONTROL will be translated to either VK_LCONTROL or VK_RCONTROL as these
                // keys are never sent on their own
                s32 actual_key = win32_key;

                // LPARAM bit 30 will be ZERO for new presses, or ONE if this is a repeat
                bool is_repeat = (lparam & 0x40000000) != 0;

                switch (win32_key)
                {
                case VK_MENU:
                    // Differentiate between left and right alt
                    if ((lparam & 0x1000000) == 0)
                    {
                        actual_key = VK_LMENU;
                        is_repeat = m_modifier_key_state[modifier_key::left_alt];
                        m_modifier_key_state[modifier_key::left_alt] = true;
                    }
                    else
                    {
                        actual_key = VK_RMENU;
                        is_repeat = m_modifier_key_state[modifier_key::right_alt];
                        m_modifier_key_state[modifier_key::right_alt] = true;
                    }
                    break;
                case VK_CONTROL:
                    // Differentiate between left and right control
                    if ((lparam & 0x1000000) == 0)
                    {
                        actual_key = VK_LCONTROL;
                        is_repeat = m_modifier_key_state[modifier_key::left_control];
                        m_modifier_key_state[modifier_key::left_control] = true;
                    }
                    else
                    {
                        actual_key = VK_RCONTROL;
                        is_repeat = m_modifier_key_state[modifier_key::right_control];
                        m_modifier_key_state[modifier_key::right_control] = true;
                    }
                    break;
                case VK_SHIFT:
                    // Differentiate between left and right shift
                    actual_key = MapVirtualKey((lparam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
                    if (actual_key == VK_LSHIFT)
                    {
                        is_repeat = m_modifier_key_state[modifier_key::left_shift];
                        m_modifier_key_state[modifier_key::left_shift] = true;
                    }
                    else
                    {
                        is_repeat = m_modifier_key_state[modifier_key::right_shift];
                        m_modifier_key_state[modifier_key::right_shift] = true;
                    }
                    break;
                case VK_CAPITAL:
                    m_modifier_key_state[modifier_key::caps_lock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
                    break;
                default:
                    // No translation needed
                    break;
                }

                // Get the character code from the virtual key pressed.  If 0, no translation from virtual key to character exists
                u32 char_code = ::MapVirtualKey(win32_key, MAPVK_VK_TO_CHAR);

                const bool result = get_message_handler()->on_key_down(actual_key, char_code, is_repeat);

                // Always return 0 to handle the message or else windows will beep
                if (result || msg != WM_SYSKEYDOWN)
                {
                    // Handled
                    return 0;
                }
            }
            break;

                // Key up
            case WM_SYSKEYUP:
            case WM_KEYUP:
            {
                // character code is stored in WPARAM
                s32 win32_key = static_cast<s32>(wparam);

                // The actual key to use.  Some keys will be translated into other keys.
                // I.E VK_CONTROL will be translated to either VK_LCONTROL or VK_RCONTROL as these
                // keys are never sent on their own
                s32 actual_key = win32_key;

                bool modifier_key_released = false;
                switch (win32_key)
                {
                case VK_MENU:
                    // Differentiate between left and right alt
                    if ((lparam & 0x1000000) == 0)
                    {
                        actual_key = VK_LMENU;
                        m_modifier_key_state[modifier_key::left_alt] = false;
                    }
                    else
                    {
                        actual_key = VK_RMENU;
                        m_modifier_key_state[modifier_key::right_alt] = false;
                    }
                    break;
                case VK_CONTROL:
                    // Differentiate between left and right control
                    if ((lparam & 0x1000000) == 0)
                    {
                        actual_key = VK_LCONTROL;
                        m_modifier_key_state[modifier_key::left_control] = false;
                    }
                    else
                    {
                        actual_key = VK_RCONTROL;
                        m_modifier_key_state[modifier_key::right_control] = false;
                    }
                    break;
                case VK_SHIFT:
                    // Differentiate between left and right shift
                    actual_key = MapVirtualKey((lparam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
                    if (actual_key == VK_LSHIFT)
                    {
                        m_modifier_key_state[modifier_key::left_shift] = false;
                    }
                    else
                    {
                        m_modifier_key_state[modifier_key::right_shift] = false;
                    }
                    break;
                case VK_CAPITAL:
                    m_modifier_key_state[modifier_key::caps_lock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
                    break;
                default:
                    // No translation needed
                    break;
                }

                // Get the character code from the virtual key pressed.  If 0, no translation from virtual key to character exists
                u32 char_code = ::MapVirtualKey(win32_key, MAPVK_VK_TO_CHAR);

                // Key up events are never repeats
                const bool is_repeat = false;

                const bool result = get_message_handler()->on_key_up(actual_key, char_code, is_repeat);

                // Note that we allow system keys to pass through to DefWndProc here, so that core features
                // like Alt+F4 to close a window work.
                if (result || msg != WM_SYSKEYUP)
                {
                    // Handled
                    return 0;
                }
            }
            break;

                // Mouse Button Down
            case WM_LBUTTONDBLCLK:
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_XBUTTONDBLCLK:
            case WM_XBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
            case WM_XBUTTONUP:
            {
                POINT cursor_point;
                cursor_point.x = (s32)(s16)LOWORD(lparam);
                cursor_point.y = (s32)(s16)HIWORD(lparam);

                ClientToScreen(hwnd, &cursor_point);

                mouse_button active_mouse_button = mouse_button::invalid;
                bool double_click = false;
                bool mouse_up = false;
                switch (msg)
                {
                case WM_LBUTTONDBLCLK:
                    double_click = true;
                    active_mouse_button = mouse_button::left;
                    break;
                case WM_LBUTTONUP:
                    mouse_up = true;
                    active_mouse_button = mouse_button::left;
                    break;
                case WM_LBUTTONDOWN:
                    active_mouse_button = mouse_button::left;
                    break;
                case WM_MBUTTONDBLCLK:
                    double_click = true;
                    active_mouse_button = mouse_button::middle;
                    break;
                case WM_MBUTTONUP:
                    mouse_up = true;
                    active_mouse_button = mouse_button::middle;
                    break;
                case WM_MBUTTONDOWN:
                    active_mouse_button = mouse_button::middle;
                    break;
                case WM_RBUTTONDBLCLK:
                    double_click = true;
                    active_mouse_button = mouse_button::right;
                    break;
                case WM_RBUTTONUP:
                    mouse_up = true;
                    active_mouse_button = mouse_button::right;
                    break;
                case WM_RBUTTONDOWN:
                    active_mouse_button = mouse_button::right;
                    break;
                case WM_XBUTTONDBLCLK:
                    double_click = true;
                    active_mouse_button = (HIWORD(wparam) & XBUTTON1) ? mouse_button::thumb01 : mouse_button::thumb02;
                    break;
                case WM_XBUTTONUP:
                    mouse_up = true;
                    active_mouse_button = (HIWORD(wparam) & XBUTTON1) ? mouse_button::thumb01  : mouse_button::thumb02;
                    break;
                case WM_XBUTTONDOWN:
                    active_mouse_button = (HIWORD(wparam) & XBUTTON1) ? mouse_button::thumb01  : mouse_button::thumb02;
                    break;
                default:
                    assert(false);
                }

                if (mouse_up)
                {
                    return get_message_handler()->on_mouse_up(active_mouse_button, cursor_point.x, cursor_point.y) ? 0 : 1;
                }
                else if (double_click)
                {
                    get_message_handler()->on_mouse_double_click(current_native_event_window, active_mouse_button, cursor_point.x, cursor_point.y);
                }
                else
                {
                    get_message_handler()->on_mouse_down(current_native_event_window, active_mouse_button, cursor_point.x, cursor_point.y);
                }
                return 0;
            }
            break;

            // Mouse Movement
            case WM_NCMOUSEMOVE:
            case WM_MOUSEMOVE:
            {
                BOOL result = false;
                return result ? 0 : 1;
            }
            break;
                // Mouse Wheel
            case WM_MOUSEWHEEL:
            {
                const f32 spin_factor = 1 / 120.0f;
                const SHORT wheel_delta = GET_WHEEL_DELTA_WPARAM(wparam);

                POINT cursor_point;
                cursor_point.x = (s32)(s16)LOWORD(lparam);
                cursor_point.y = (s32)(s16)HIWORD(lparam);

                const BOOL result = get_message_handler()->on_mouse_wheel(static_cast<f32>(wheel_delta) * spin_factor, cursor_point.x, cursor_point.y);
                return result ? 0 : 1;
            }
            break;

                // Window focus and activation
            case WM_MOUSEACTIVATE:
            {
                // If the mouse activate isn't in the client area we'll force the WM_ACTIVATE to be EWindowActivation::ActivateByMouse
                // This ensures that clicking menu buttons on the header doesn't generate a WM_ACTIVATE with EWindowActivation::Activate
                // which may cause mouse capture to be taken because is not differentiable from Alt-Tabbing back to the application.
                m_force_activate_by_mouse = !(LOWORD(lparam) & HTCLIENT);
                return 0;
            }
            break;

                // Window focus and activation
            case WM_ACTIVATE:
            {
                window_activation activation_type;

                if (LOWORD(wparam) & WA_ACTIVE)
                {
                    activation_type = m_force_activate_by_mouse ? window_activation::activate_by_mouse : window_activation::activate;
                }
                else if (LOWORD(wparam) & WA_CLICKACTIVE)
                {
                    activation_type = window_activation::activate_by_mouse;
                }
                else
                {
                    activation_type = window_activation::deactivate;
                }
                m_force_activate_by_mouse = false;

                update_all_modifier_key_states();

                if (current_native_event_window)
                {
                    BOOL result = false;
                    result = get_message_handler()->on_window_activation_changed(current_native_event_window, activation_type);
                    return result ? 0 : 1;
                }

                return 1;
            }
            break;

            case WM_ACTIVATEAPP:
                update_all_modifier_key_states();
                get_message_handler()->on_application_activation_changed(!!wparam);
                break;

            case WM_NCACTIVATE:
            {
                if (current_native_event_window && !current_native_event_window->get_definition().has_os_window_border)
                {
                    // Unless using the OS window border, intercept calls to prevent non-client area drawing a border upon activation or deactivation
                    // Return true to ensure standard activation happens
                    return true;
                }
            }
            break;

            case WM_NCPAINT:
            {
                if (current_native_event_window && !current_native_event_window->get_definition().has_os_window_border)
                {
                    // Unless using the OS window border, intercept calls to draw the non-client area - we do this ourselves
                    return 0;
                }
            }
            break;

            case WM_CLOSE:
            {
                if (current_native_event_window)
                {
                    // Called when the OS close button is pressed
                    get_message_handler()->on_window_close(current_native_event_window);
                }
                return 0;
            }
            break;

            case WM_SHOWWINDOW:
            break;

            case WM_SIZE:
            {
                if (current_native_event_window)
                {
                    // @todo Fullscreen - Perform deferred resize
                    // Note WM_SIZE provides the client dimension which is not equal to the window dimension if there is a windows border
                    const s32 new_width = (s32)(s16)(LOWORD(lparam));
                    const s32 new_height = (s32)(s16)(HIWORD(lparam));

                    const generic_window_definition& definition = current_native_event_window->get_definition();
                    if (definition.is_regular_window && !definition.has_os_window_border)
                    {
                        current_native_event_window->adjust_window_region(new_width, new_height);
                    }

                    const bool b_was_minimized = (wparam == SIZE_MINIMIZED);
                    const bool b_is_fullscreen = (current_native_event_window->get_window_mode() == window_mode::fullscreen);

                    // When in fullscreen m_windows rendering size should be determined by the application. Do not adjust based on WM_SIZE messages.
                    if (!b_is_fullscreen)
                    {
                        const bool result = get_message_handler()->on_size_changed(current_native_event_window, new_width, new_height, b_was_minimized);
                    }
                }
            }
            break;
            case WM_SIZING:
            {
                if (current_native_event_window)
                {
                    get_message_handler()->on_resizing_window(current_native_event_window);
                }
            }
            break;
            case WM_ENTERSIZEMOVE:
            {
                if (current_native_event_window)
                {
                    get_message_handler()->begin_reshaping_window(current_native_event_window);
                }
            }
            break;
            case WM_EXITSIZEMOVE:
            {
                if (current_native_event_window)
                {
                    get_message_handler()->finished_reshaping_window(current_native_event_window);
                }
            }
            break;
            }
        }

        return 0;
    }
}