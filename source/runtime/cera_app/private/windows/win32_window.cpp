#include "windows/win32_window.h"
#include "windows/win32_application.h"
#include "windows/win32_application_helpers.h"

#include "util/log.h"

#include <dwmapi.h>

namespace cera
{
    const tchar windows_window::s_app_window_class[] = L"Cera Window";

    std::shared_ptr<windows_window> windows_window::make()
    {
        // First, allocate the new native window object.  This doesn't actually create a native window or anything,
        // we're simply instantiating the object so that we can keep shared references to it.
        return std::make_shared<windows_window>();
    }

    windows_window::windows_window()
        : m_hwnd(NULL)
        , m_window_mode(window_mode::windowed)
        , m_aspect_ratio(1.0f)
        , m_is_visible(false)
        , m_is_first_time_visible(true)
        , m_initially_minimized(false)
        , m_initially_maximized(false)
    {
        // PreFullscreenWindowPlacement.length will be set when we save the window placement and then used to check if the structure is valid
        std::memset(&m_pre_fullscreen_window_placement, 0, sizeof(WINDOWPLACEMENT));

        std::memset(&m_pre_parent_minimized_window_placement, 0, sizeof(WINDOWPLACEMENT));
        m_pre_parent_minimized_window_placement.length = sizeof(WINDOWPLACEMENT);
    }

    windows_window::~windows_window()
    {
        // NOTE: The m_hwnd is invalid here!
        //       Doing stuff with HWnds will fail silently.
        //       Use Destroy() instead.
    }
    
    HWND windows_window::get_hwnd() const
    {
        return m_hwnd;
    }

    void windows_window::initialize(windows_application* const application, const generic_window_definition& definition, HINSTANCE hinstance, const bool show)
    {
        set_definition(definition);

        m_owning_application = application;

        // Finally, let's initialize the new native window object.  Calling this function will often cause OS
        // window messages to be sent! (such as activation messages)
        u32 window_ex_style = 0;
        u32 window_style = 0;

        m_region_width = -1; 
        m_region_height = -1;

        const f32 x_initial_rect = get_definition().x_desired_position_on_screen;
        const f32 y_initial_rect = get_definition().y_desired_position_on_screen;

        const f32 width_initial = get_definition().width_desired_on_screen;
        const f32 height_initial = get_definition().height_desired_on_screen;

        s32 client_x = static_cast<s32>(x_initial_rect);
        s32 client_y = static_cast<s32>(y_initial_rect);
        s32 client_width = static_cast<s32>(width_initial);
        s32 client_height = static_cast<s32>(height_initial);
        s32 window_x = client_x;
        s32 window_y = client_y;
        s32 window_width = client_width;
        s32 window_height = client_height;

        if(get_definition().center_behaviour == window_center_behaviour::auto_center)
        {
            s32 screen_width = ::GetSystemMetrics(SM_CXSCREEN);
            s32 screen_height = ::GetSystemMetrics(SM_CYSCREEN);

            // Center the window within the screen. Clamp to 0, 0 for the top-left corner.
            window_x = std::max<s32>(0, (screen_width - window_width) / 2);
            window_y = std::max<s32>(0, (screen_height - window_height) / 2);
        }

        if (!get_definition().has_os_window_border)
        {
            window_ex_style = WS_EX_WINDOWEDGE;
            window_style = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

            if (get_definition().appears_in_taskbar)
            {
                window_ex_style |= WS_EX_APPWINDOW;
            }
            else
            {
                window_ex_style |= WS_EX_TOOLWINDOW;
            }

            if (get_definition().is_topmost_window)
            {
                // Tool tips are always top most windows
                window_ex_style |= WS_EX_TOPMOST;
            }

            if (!get_definition().accepts_input)
            {
                // Window should never get input
                window_ex_style |= WS_EX_TRANSPARENT;
            }
        }
        else
        {
            // OS Window border setup
            window_ex_style = WS_EX_APPWINDOW;
            window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;

            if (is_regular_window())
            {
                if (get_definition().supports_maximize)
                {
                    window_style |= WS_MAXIMIZEBOX;
                }

                if (get_definition().supports_minimize)
                {
                    window_style |= WS_MINIMIZEBOX;
                }

                if (get_definition().has_sizing_frame)
                {
                    window_style |= WS_THICKFRAME;
                }
                else
                {
                    window_style |= WS_BORDER;
                }
            }
            else
            {
                window_style |= WS_POPUP | WS_BORDER;
            }

            // x,y, width, height defines the top-left pixel of the client area on the screen
            // This adjusts a zero rect to give us the size of the border
            RECT border_rect = {0, 0, 0, 0};
            ::AdjustWindowRectEx(&border_rect, window_style, FALSE, window_ex_style);

            // Border rect size is negative - see MoveWindowTo
            window_x += border_rect.left;
            window_y += border_rect.top;

            // Inflate the window size by the OS border
            window_width += border_rect.right - border_rect.left;
            window_height += border_rect.bottom - border_rect.top;
        }

        // Creating the Window
        m_hwnd = ::CreateWindowEx(
            window_ex_style,
            s_app_window_class,
            get_definition().title.c_str(),
            window_style,
            window_x,
            window_y,
            window_width,
            window_height,
            NULL,
            NULL,
            hinstance,
            NULL);

        if (m_hwnd == NULL)
        {
            log::error("Window Creation Failed (Error Code %d).", GetLastError());
            return;
        }

        // We call reshape window here because we didn't take into account the non-client area
        // in the initial creation of the window. Reshape window may resize the window if the
        // non-client area is encroaching on our desired client area space.
        reshape_window(window_x, window_y, client_width, client_height);

        // Disable DWM Rendering and Nonclient Area painting if not showing the os window border
        // This prevents the standard windows frame from ever being drawn
        if (!get_definition().has_os_window_border)
        {
            const DWMNCRENDERINGPOLICY rendering_policy = DWMNCRP_DISABLED;
            if(FAILED(::DwmSetWindowAttribute(m_hwnd, DWMWA_NCRENDERING_POLICY, &rendering_policy, sizeof(rendering_policy))))
            {
                log::error("DwmSetWindowAttribute -> DWMWA_NCRENDERING_POLICY: failed");
                return;
            }

            const BOOL enable_nc_paint = FALSE;
            if(FAILED(::DwmSetWindowAttribute(m_hwnd, DWMWA_ALLOW_NCPAINT, &enable_nc_paint, sizeof(enable_nc_paint))))
            {
                log::error("DwmSetWindowAttribute -> DWMWA_ALLOW_NCPAINT: failed");
                return;
            }
        }

        // No region for non regular windows or windows displaying the os window border
        if (is_regular_window() && !get_definition().has_os_window_border)
        {
            window_style |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
            if (get_definition().supports_maximize)
            {
                window_style |= WS_MAXIMIZEBOX;
            }
            if (get_definition().supports_minimize)
            {
                window_style |= WS_MINIMIZEBOX;
            }
            if (get_definition().has_sizing_frame)
            {
                window_style |= WS_THICKFRAME;
            }

            if(!SetWindowLong(m_hwnd, GWL_STYLE, (LONG)window_style))
            {
                log::error("Failed to set GWL_STYLE on window");
                return;
            }

            u32 set_window_position_flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED;

            if (get_definition().activation_policy == window_activation_policy::never)
            {
                set_window_position_flags |= SWP_NOACTIVATE;
            }

            ::SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0, set_window_position_flags);

            // For regular non-game windows delete the close menu from the default system menu. This prevents accidental closing of win32 apps by double clicking by accident on the application icon
            // The overwhelming majority of feedback is that is confusing behavior and we want to prevent this.
            ::DeleteMenu(GetSystemMenu(m_hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND);

            adjust_window_region(client_width, client_height);
        }
        else if (get_definition().has_os_window_border)
        {
            if (!get_definition().has_close_button)
            {
                EnableMenuItem(GetSystemMenu(m_hwnd, FALSE), SC_CLOSE, MF_GRAYED);
            }
        }
    }

    bool windows_window::is_regular_window() const
    {
        return get_definition().is_regular_window;
    }

    void windows_window::adjust_window_region(s32 width, s32 height)
    {
        m_region_width = width;
        m_region_height = height;

        HRGN region = make_window_region_object (true);

        // NOTE: We explicitly don't delete the region object, because the OS deletes the handle when it no longer needed after
        // a call to SetWindowRgn.
        if(!::SetWindowRgn(m_hwnd, region, FALSE))
        {
            log::error("Failed to adjust window region");
            return;
        }
    }

    f32 windows_window::get_aspect_ratio() const
    {
        return m_aspect_ratio;
    }

    /** @return True if the window is enabled */
    bool windows_window::is_enabled()
    {
        return !!::IsWindowEnabled(m_hwnd);
    }

    void windows_window::reshape_window(s32 new_x, s32 new_y, s32 new_width, s32 new_height)
    {
        WINDOWINFO window_info;
        memset( &window_info, 0, sizeof(WINDOWINFO) );
        window_info.cbSize = sizeof(window_info);
        ::GetWindowInfo(m_hwnd, &window_info);

        m_aspect_ratio = (f32)new_width / (f32)new_height;

        // x,y, width, height defines the top-left pixel of the client area on the screen
        if (get_definition().has_os_window_border)
        {
            // This adjusts a zero rect to give us the size of the border
            RECT border_rect = {0, 0, 0, 0};
            ::AdjustWindowRectEx(&border_rect, window_info.dwStyle, FALSE, window_info.dwExStyle);

            // Border rect size is negative - see MoveWindowTo
            new_x += border_rect.left;
            new_y += border_rect.top;

            // Inflate the window size by the OS border
            new_width += border_rect.right - border_rect.left;
            new_height += border_rect.bottom - border_rect.top;
        }

        // the window position is the requested position
        s32 window_x = new_x;
        s32 window_y = new_y;

        if (is_maximized())
        {
            restore();
        }

        // We use SWP_NOSENDCHANGING when in fullscreen mode to prevent Windows limiting our window size to the current resolution, as that
        // prevents us being able to change to a higher resolution while in fullscreen mode
        ::SetWindowPos(m_hwnd, nullptr, window_x, window_y, new_width, new_height, SWP_NOZORDER | SWP_NOACTIVATE | ((m_window_mode == window_mode::fullscreen) ? SWP_NOSENDCHANGING : 0));

        bool adjust_corners = get_definition().corner_radius > 0;

        if (!get_definition().has_os_window_border && adjust_corners)
        {
            adjust_window_region(new_width, new_height);
        }
    }

    bool windows_window::get_full_screen_info(s32& x, s32& y, s32& width, s32& height) const
    {
        bool true_fullscreen = (m_window_mode == window_mode::fullscreen);

        // Grab current monitor data for sizing
        HMONITOR monitor = MonitorFromWindow(m_hwnd, true_fullscreen ? MONITOR_DEFAULTTOPRIMARY : MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitor_info;
        monitor_info.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(monitor, &monitor_info);

        x = monitor_info.rcMonitor.left;
        y = monitor_info.rcMonitor.top;
        width = monitor_info.rcMonitor.right - x;
        height = monitor_info.rcMonitor.bottom - y;

        return true;
    }

    /** Native windows should implement MoveWindowTo by relocating the client area of the platform-specific window to (x,y). */
    void windows_window::move_window_to(s32 x, s32 y)
    {
        // GUI gives the window position as relative to the client area of a window, so we may need to compensate for the OS border
        if (get_definition().has_os_window_border)
        {
            const LONG window_style = ::GetWindowLong(m_hwnd, GWL_STYLE);
            const LONG window_ex_style = ::GetWindowLong(m_hwnd, GWL_EXSTYLE);

            // This adjusts a zero rect to give us the size of the border
            RECT border_rect = {0, 0, 0, 0};
            ::AdjustWindowRectEx(&border_rect, window_style, FALSE, window_ex_style);

            // Border rect size is negative
            x += border_rect.left;
            y += border_rect.top;
        }

        ::SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
    }

    /** Native windows should implement BringToFront by making this window the top-most window (i.e. focused).
     *
     * @param force	Forces the window to the top of the Z order, even if that means stealing focus from other windows
     *					In general do not pass true for this.  It is really only useful for some windows, like game windows where not forcing it to the front
     *					could cause mouse capture and mouse lock to happen but without the window visible
     */
    void windows_window::bring_to_front(bool force)
    {
        if (is_regular_window())
        {
            if (::IsIconic(m_hwnd))
            {
                ::ShowWindow(m_hwnd, SW_RESTORE);
            }
            else
            {
                ::SetActiveWindow(m_hwnd);
            }
        }
        else
        {
            HWND hwnd_insert_after = HWND_TOP;

            // By default we activate the window or it isn't actually brought to the front
            u32 flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER;

            if (!force)
            {
                flags |= SWP_NOACTIVATE;
            }

            if (get_definition().is_topmost_window)
            {
                hwnd_insert_after = HWND_TOPMOST;
            }

            ::SetWindowPos(m_hwnd, hwnd_insert_after, 0, 0, 0, 0, flags);
        }
    }

    /** Native windows should implement this function by asking the OS to destroy OS-specific resource associated with the window (e.g. Win32 window handle) */
    void windows_window::destroy()
    {
        ::DestroyWindow(m_hwnd);
    }

    /** Native window should implement this function by performing the equivalent of the Win32 minimize-to-taskbar operation */
    void windows_window::minimize()
    {
        // Windows window handles the initial show state on the first Show call in order to handle activation policy.
        // So we only call now if that already happened.
        if (!m_is_first_time_visible)
        {
            ::ShowWindow(m_hwnd, SW_MINIMIZE);
        }
        else
        {
            m_initially_minimized = true;
            m_initially_maximized = false;
        }
    }

    /** Native window should implement this function by performing the equivalent of the Win32 maximize operation */
    void windows_window::maximize()
    {
        // Windows window handles the initial show state on the first Show call in order to handle activation policy.
        // So we only call now if that already happened.
        if (!m_is_first_time_visible)
        {
            ::ShowWindow(m_hwnd, SW_MAXIMIZE);
        }
        else
        {
            m_initially_maximized = true;
            m_initially_minimized = false;
        }
    }

    /** Native window should implement this function by performing the equivalent of the Win32 maximize operation */
    void windows_window::restore()
    {
        // Windows window handles the initial show state on the first Show call in order to handle activation policy.
        // So we only call now if that already happened.
        if (!m_is_first_time_visible)
        {
            ::ShowWindow(m_hwnd, SW_RESTORE);
        }
        else
        {
            m_initially_maximized = false;
            m_initially_minimized = false;
        }
    }

    /** Native window should make itself visible */
    void windows_window::show()
    {
        if (!m_is_visible)
        {
            m_is_visible = true;

            // Should the show command include activation?
            // Do not activate windows that do not take input; e.g. tool-tips and cursor decorators
            bool should_activate = false;
            if (get_definition().accepts_input)
            {
                should_activate = get_definition().activation_policy == window_activation_policy::always;
                if (m_is_first_time_visible && get_definition().activation_policy == window_activation_policy::first_shown)
                {
                    should_activate = true;
                }
            }

            // Map to the relevant ShowWindow command.
            s32 show_window_command = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
            if (m_is_first_time_visible)
            {
                m_is_first_time_visible = false;
                if (m_initially_minimized)
                {
                    show_window_command = should_activate ? SW_MINIMIZE : SW_SHOWMINNOACTIVE;
                }
                else if (m_initially_maximized)
                {
                    show_window_command = should_activate ? SW_SHOWMAXIMIZED : SW_MAXIMIZE;
                }
            }

            ::ShowWindow(m_hwnd, show_window_command);

            // Turns out SW_SHOWNA doesn't work correctly if the window has never been shown before.  If the window
            // was already maximized, (and hidden) and we're showing it again, SW_SHOWNA would be right.  But it's not right
            // to use SW_SHOWNA when the window has never been shown before!
            //
            // TODO Add in a more complicated path that involves SW_SHOWNA if we hide windows in their maximized/minimized state.
            //::ShowWindow(m_hwnd, should_activate ? SW_SHOW : SW_SHOWNA);
        }
    }

    /** Native window should hide itself */
    void windows_window::hide()
    {
        if (m_is_visible)
        {
            m_is_visible = false;
            ::ShowWindow(m_hwnd, SW_HIDE);
        }
    }

    /** Toggle native window between fullscreen and normal mode */
    void windows_window::set_window_mode(window_mode new_window_mode)
    {
        if (new_window_mode != m_window_mode)
        {
            window_mode prev_window_mode = m_window_mode;
            m_window_mode = new_window_mode;

            const bool true_fullscreen = new_window_mode == window_mode::fullscreen;

            // Setup Win32 flags to be used for fullscreen mode
            LONG window_style = GetWindowLong(m_hwnd, GWL_STYLE);
            const LONG fullscreen_mode_style = WS_POPUP;

            LONG window_mode_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
            if (is_regular_window())
            {
                if (get_definition().supports_maximize)
                {
                    window_mode_style |= WS_MAXIMIZEBOX;
                }

                if (get_definition().supports_minimize)
                {
                    window_mode_style |= WS_MINIMIZEBOX;
                }

                if (get_definition().has_sizing_frame)
                {
                    window_mode_style |= WS_THICKFRAME;
                }
                else
                {
                    window_mode_style |= WS_BORDER;
                }
            }
            else
            {
                window_mode_style |= WS_POPUP | WS_BORDER;
            }

            // If we're not in fullscreen, make it so
            if (new_window_mode == window_mode::windowed_fullscreen || new_window_mode == window_mode::fullscreen)
            {
                if (prev_window_mode == window_mode::windowed)
                {
                    m_pre_fullscreen_window_placement.length = sizeof(WINDOWPLACEMENT);
                    ::GetWindowPlacement(m_hwnd, &m_pre_fullscreen_window_placement);
                }

                // Setup Win32 flags for fullscreen window
                window_style &= ~window_mode_style;
                window_style |= fullscreen_mode_style;

                SetWindowLong(m_hwnd, GWL_STYLE, window_style);
                ::SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                if (!true_fullscreen)
                {
                    // Ensure the window is restored if we are going for WindowedFullscreen
                    ShowWindow(m_hwnd, SW_RESTORE);
                }

                // Get the current window position.
                RECT client_rect;
                GetClientRect(m_hwnd, &client_rect);

                // Grab current monitor data for sizing
                HMONITOR monitor = MonitorFromWindow(m_hwnd, true_fullscreen ? MONITOR_DEFAULTTOPRIMARY : MONITOR_DEFAULTTONEAREST);
                MONITORINFO monitor_info;
                monitor_info.cbSize = sizeof(MONITORINFO);
                GetMonitorInfo(monitor, &monitor_info);

                // Get the target client width to send to ReshapeWindow.
                // Preserve the current res if going to true fullscreen and the monitor supports it and allow the calling code
                // to resize if required.
                // Else, use the monitor's res for windowed fullscreen.
                LONG monitor_width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
                LONG target_client_width = true_fullscreen ? (std::min)(monitor_width, client_rect.right - client_rect.left) : monitor_width;

                LONG monitor_height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
                LONG target_client_height = true_fullscreen ? (std::min)(monitor_height, client_rect.bottom - client_rect.top) : monitor_height;

                // Resize and position fullscreen window
                reshape_window(
                    monitor_info.rcMonitor.left,
                    monitor_info.rcMonitor.top,
                    target_client_width,
                    target_client_height);
            }
            else
            {
                // When windowed
                // Setup Win32 flags for restored window
                window_style &= ~fullscreen_mode_style;
                window_style |= window_mode_style;
                SetWindowLong(m_hwnd, GWL_STYLE, window_style);
                ::SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                if (m_pre_fullscreen_window_placement.length != 0) // Was PreFullscreenWindowPlacement initialized?
                {
                    ::SetWindowPlacement(m_hwnd, &m_pre_fullscreen_window_placement);
                }

                // Set the icon back again as it seems to get ignored if the application has ever started in full screen mode
                HICON hicon = (HICON)::GetClassLongPtr(m_hwnd, GCLP_HICON);
                if (hicon != nullptr)
                {
                    ::SendMessageW(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
                }
            }
        }
    }

    window_mode windows_window::get_window_mode() const
    {
        return m_window_mode;
    }

    /** @return true if the native window is maximized, false otherwise */
    bool windows_window::is_maximized() const
    {
        bool is_maximized = !!::IsZoomed(m_hwnd);
        return is_maximized;
    }

    /** @return true if the native window is minimized, false otherwise */
    bool windows_window::is_minimized() const
    {
        return !!::IsIconic(m_hwnd);
    }

    /** @return true if the native window is visible, false otherwise */
    bool windows_window::is_visible() const
    {
        return m_is_visible;
    }

    /** Returns the size and location of the window when it is restored */
    bool windows_window::get_restored_dimensions(s32& x, s32& y, s32& width, s32& height)
    {
        WINDOWPLACEMENT window_placement;
        window_placement.length = sizeof(WINDOWPLACEMENT);

        if (::GetWindowPlacement(m_hwnd, &window_placement) != 0)
        {
            // This window may be minimized.  Get the position when it is restored.
            const RECT restored = window_placement.rcNormalPosition;

            x = restored.left;
            y = restored.top;
            width = restored.right - restored.left;
            height = restored.bottom - restored.top;

            const LONG window_ex_style = ::GetWindowLong(m_hwnd, GWL_EXSTYLE);
            if ((window_ex_style & WS_EX_TOOLWINDOW) == 0)
            {
                // For windows without WS_EX_TOOLWINDOW window style window_placement.rcNormalPosition is in workspace coordinates, so we need to convert the position to screen coordinates
                const bool true_fullscreen = (m_window_mode == window_mode::fullscreen);
                HMONITOR monitor = MonitorFromWindow(m_hwnd, true_fullscreen ? MONITOR_DEFAULTTOPRIMARY : MONITOR_DEFAULTTONEAREST);
                MONITORINFO monitor_info;
                monitor_info.cbSize = sizeof(MONITORINFO);
                GetMonitorInfo(monitor, &monitor_info);

                x += monitor_info.rcWork.left - monitor_info.rcMonitor.left;
                y += monitor_info.rcWork.top - monitor_info.rcMonitor.top;
            }

            return true;
        }

        return false;
    }

    /** Sets focus on the native window */
    void windows_window::set_window_focus()
    {
        if (GetFocus() != m_hwnd)
        {
            ::SetFocus(m_hwnd);
        }
    }

    /**
     * Sets the opacity of this window
     *
     * @param	InOpacity	The new window opacity represented as a floating point scalar
     */
    void windows_window::set_opacity(const f32 opacity)
    {
        SetLayeredWindowAttributes(m_hwnd, 0, (BYTE)static_cast<s32>(opacity * 255.0f), LWA_ALPHA);
    }

    /**
     * Enables or disables this window.  When disabled, a window will receive no input.
     *
     * @param bEnable	true to enable the window, false to disable it.
     */
    void windows_window::enable(bool enable)
    {
        ::EnableWindow(m_hwnd, (BOOL)enable);
    }

    /** @return true if native window exists underneath the coordinates */
    bool windows_window::is_point_in_window(s32 x, s32 y) const
    {
        bool result = false;

        HRGN region = make_window_region_object (false);
        result = !!PtInRegion(region, x, y);
        DeleteObject(region);

        return result;
    }

    s32 windows_window::get_window_border_size() const
    {
        if (get_definition().type == window_type::game_window && !get_definition().has_os_window_border)
        {
            // Our borderless game windows actually have a thick border to allow sizing, which we draw over to simulate
            // a borderless window. We return zero here so that the game will correctly behave as if this is truly a
            // borderless window.
            return 0;
        }

        WINDOWINFO window_info;
        memset(&window_info, 0, sizeof(WINDOWINFO));
        window_info.cbSize = sizeof(window_info);
        ::GetWindowInfo(m_hwnd, &window_info);

        return (s32)window_info.cxWindowBorders;
    }

    s32 windows_window::get_window_title_bar_size() const
    {
        return GetSystemMetrics(SM_CYCAPTION);
    }

    void* windows_window::get_os_window_handle() const
    {
        return m_hwnd;
    }

    bool windows_window::is_foreground_window() const
    {
        return ::GetForegroundWindow() == m_hwnd;
    }

    bool windows_window::is_fullscreen_supported() const
    {
        // fullscreen not supported when using remote desktop
        return !::GetSystemMetrics(SM_REMOTESESSION);
    }

    void windows_window::set_text(const tchar *const in_text)
    {
        SetWindowText(m_hwnd, in_text);
    }

    HRGN windows_window::make_window_region_object(bool include_border_when_maximized) const
    {
        HRGN region;
        if (m_region_width != -1 && m_region_height != -1)
        {
            const bool is_borderless_game_window = get_definition().type == window_type::game_window && !get_definition().has_os_window_border;
            if (is_maximized())
            {
                if (is_borderless_game_window)
                {
                    // Windows caches the cxWindowBorders size at window creation. Even if borders are removed or resized Windows will continue to use this value when evaluating regions
                    // and sizing windows. When maximized this means that our window position will be offset from the screen origin by (-cxWindowBorders,-cxWindowBorders). We want to
                    // display only the region within the maximized screen area, so offset our upper left and lower right by cxWindowBorders.
                    WINDOWINFO window_info;
                    memset(&window_info, 0, sizeof(WINDOWINFO));
                    window_info.cbSize = sizeof(window_info);
                    ::GetWindowInfo(m_hwnd, &window_info);

                    const s32 window_border_size = include_border_when_maximized ? (s32)window_info.cxWindowBorders : 0;
                    region = CreateRectRgn(window_border_size, window_border_size, m_region_width + window_border_size, m_region_height + window_border_size);
                }
                else
                {
                    const s32 window_border_size = include_border_when_maximized ? get_window_border_size() : 0;
                    region = CreateRectRgn(window_border_size, window_border_size, m_region_width - window_border_size, m_region_height - window_border_size);
                }
            }
            else
            {
                const bool use_corner_radius = m_window_mode == window_mode::windowed && !is_borderless_game_window &&
                                              get_definition().corner_radius > 0;

                if (use_corner_radius)
                {
                    // CreateRoundRectRgn gives you a duff region that's 1 pixel smaller than you ask for. CreateRectRgn behaves correctly.
                    // This can be verified by uncommenting the assert below
                    region = CreateRoundRectRgn(0, 0, m_region_width + 1, m_region_height + 1, get_definition().corner_radius, get_definition().corner_radius);

                    // Test that a point that should be in the region, is in the region
                    // check(!!PtInRegion(region, m_region_width-1, m_region_height/2));
                }
                else
                {
                    region = CreateRectRgn(0, 0, m_region_width, m_region_height);
                }
            }
        }
        else
        {
            RECT rc_wnd;
            GetWindowRect(m_hwnd, &rc_wnd);
            region = CreateRectRgn(0, 0, rc_wnd.right - rc_wnd.left, rc_wnd.bottom - rc_wnd.top);
        }

        return region;
    }
}