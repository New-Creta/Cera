#pragma once

#include "util/types.h"

#include "windows/win32_min.h"

#include "generic_window.h"
#include "generic_window_definition.h"

#include <memory>

namespace cera
{
    class windows_application;

    /**
     * A platform specific implementation of FNativeWindow.
     *
     * Native Windows provide platform-specific backing for and are always owned by an SWindow.
     */
    class windows_window : public generic_window
    {
    public:
        /** Win32 requirement: see CreateWindowEx and RegisterClassEx. */
        static const tchar s_app_window_class[];

        /** Create a new FWin32Window. */
        static std::shared_ptr<windows_window> make();

    public:
        /** The constructor */
        windows_window();

        /** Destructor. */
        ~windows_window();

        /**
         * Gets the Window's handle.
         *
         * @return The window's HWND handle.
         */
        HWND get_hwnd() const;

        /**
         * Initialize a window with a given definition
        */
        void initialize(class windows_application* const application, const generic_window_definition& definition, HINSTANCE hinstance, const bool show);

        /**
         * @return If this window is a regular window ( so not a Tooltip or a Menuitem etc. )
         */
        bool is_regular_window() const;

        /**
         * Sets the window region to specified dimensions.
         *
         * @param width The width of the window region (in pixels).
         * @param height The height of the window region (in pixels).
         */
        void adjust_window_region(s32 width, s32 height);

        /** @return Get window aspect ration ( width / height ) */
        float get_aspect_ratio() const;

        /** @return True if the window is enabled */
        bool is_enabled();

    public:
        // generic_window interface

        void reshape_window(s32 x, s32 y, s32 width, s32 height) override;
        bool get_full_screen_info(s32& x, s32& y, s32& width, s32& height) const override;
        void move_window_to(s32 x, s32 y) override;
        void bring_to_front(bool force = false) override;
        void destroy() override;
        void minimize() override;
        void maximize() override;
        void restore() override;
        void show() override;
        void hide() override;
        void set_window_mode(window_mode new_window_mode) override;
        window_mode get_window_mode() const override;
        bool is_maximized() const override;
        bool is_minimized() const override;
        bool is_visible() const override;
        bool get_restored_dimensions(s32& x, s32& y, s32& width, s32& height) override;
        void set_window_focus() override;
        void set_opacity(const f32 opacity);
        void enable(bool enable) override;
        bool is_point_in_window(s32 x, s32 y) const override;
        s32 get_window_border_size() const override;
        s32 get_window_title_bar_size() const override;
        void* get_os_window_handle() const override;
        bool is_foreground_window() const override;
        bool is_fullscreen_supported() const override;
        void set_text(const tchar* const in_text) override;

    private:
        /** Creates an HRGN for the window's current region.  Remember to delete this when you're done with it using
           ::DeleteObject, unless you're passing it to SetWindowRgn(), which will absorb the reference itself. */
        HRGN make_window_region_object(bool include_border_when_maximized) const;

    private:
        /** The application that owns this window. */
        windows_application* m_owning_application;

        /** The window's handle. */
        HWND m_hwnd;

        /** Store the window region size for querying whether a point lies within the window. */
        s32 m_region_width;
        s32 m_region_height;

        /** The mode that the window is in (windowed, fullscreen, windowedfullscreen ) */
        window_mode m_window_mode;

        /** The placement of the window before it entered a fullscreen state. */
        WINDOWPLACEMENT m_pre_fullscreen_window_placement;

        /** The placement of the window before it entered a minimized state due to its parent window being minimized. */
        WINDOWPLACEMENT m_pre_parent_minimized_window_placement;

        /** Current aspect ratio of window's client area */
        float m_aspect_ratio;

        /** Whether the window is currently shown */
        bool m_is_visible;

        /** Whether the window is yet to have its first Show() call. This is set false after first Show(). */
        bool m_is_first_time_visible;

        /** We cache the min/max state for any Minimize, Maximize, or Restore calls that were made before the first Show */
        bool m_initially_minimized;
        bool m_initially_maximized;
    };
}