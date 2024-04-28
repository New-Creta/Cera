#pragma once

#include "util/types.h"

#include "generic_window_definition.h"

#include <memory>

namespace cera
{
    /**
     * Modes that an generic window can be in
     */
    enum class window_mode
    {
        /** The window is in true fullscreen mode */
        fullscreen,
        /** The window has no border and takes up the entire area of the screen */
        windowed_fullscreen,
        /** The window has a border and may not take up the entire screen area */
        windowed
    };

    class generic_window
    {
    public:
        generic_window();

        virtual ~generic_window();

        /** Native windows should implement ReshapeWindow by changing the platform-specific window to be located at (x,y) and be the dimensions width x height. */
        virtual void reshape_window(s32 x, s32 y, s32 width, s32 height);

        /** Returns the rectangle of the screen the window is associated with */
        virtual bool get_full_screen_info(s32& x, s32& y, s32& width, s32& height) const;

        /** Native windows should implement MoveWindowTo by relocating the platform-specific window to (x,y). */
        virtual void move_window_to(s32 x, s32 y);

        /** Native windows should implement BringToFront by making this window the top-most window (i.e. focused). */
        virtual void bring_to_front(bool force = false);

        /** Native windows should implement this function by asking the OS to destroy OS-specific resource associated with the window (e.g. Win32 window handle) */
        virtual void destroy();

        /** Native window should implement this function by performing the equivalent of the Win32 minimize-to-taskbar operation */
        virtual void minimize();

        /** Native window should implement this function by performing the equivalent of the Win32 maximize operation */
        virtual void maximize();

        /** Native window should implement this function by performing the equivalent of the Win32 restore operation */
        virtual void restore();

        /** Native window should make itself visible */
        virtual void show();

        /** Native window should hide itself */
        virtual void hide();

        /** Toggle native window between fullscreen and normal mode */
        virtual void set_window_mode(window_mode new_window_mode);

        /** @return true if the native window is currently in fullscreen mode, false otherwise */
        virtual window_mode get_window_mode() const;

        /** @return true if the native window is maximized, false otherwise */
        virtual bool is_maximized() const;

        /** @return true if the native window is minimized, false otherwise */
        virtual bool is_minimized() const;

        /** @return true if the native window is visible, false otherwise */
        virtual bool is_visible() const;

        /**
         * Populates the size and location of the window when it is restored.
         * If the function fails, false is returned and x,y,width,height will be undefined.
         *
         * @return true when the size and location and successfully retrieved; false otherwise.
         */
        virtual bool get_restored_dimensions(s32& x, s32& y, s32& width, s32& height);

        /**
         * Native windows should implement SetWindowFocus to let the OS know that a window has taken focus.
         * Slate handles focus on a per widget basis internally but the OS still needs to know what window has focus for proper message routing
         */
        virtual void set_window_focus();

        /**
         * Sets the opacity of this window
         *
         * @param	in_opacity	The new window opacity represented as a floating point scalar
         */
        virtual void set_opacity(const float in_opacity);

        /**
         * Enables or disables the window.  If disabled the window receives no input
         *
         * @param enable	true to enable the window, false to disable it.
         */
        virtual void enable(bool enable);

        /** @return true if native window exists underneath the coordinates */
        virtual bool is_point_in_window(s32 x, s32 y) const;

        /** Gets OS specific window border size. This is necessary because Win32 does not give control over this size. */
        virtual s32 get_window_border_size() const;

        /** Gets OS specific window title bar size */
        virtual s32 get_window_title_bar_size() const;

        /** Gets the OS Window handle in the form of a void pointer for other API's */
        virtual void *get_os_window_handle() const;

        /** @return true if the window is in the foreground */
        virtual bool is_foreground_window() const;

        /** @return true if the window is in the foreground */
        virtual bool is_fullscreen_supported() const;

        /**
         * Sets the window text - usually the title but can also be text content for things like controls
         *
         * @param Text	The window's title or content text
         */
        virtual void set_text(const tchar *const Text);

        /** @return	The definition describing properties of the window */
        virtual const generic_window_definition& get_definition() const;

    protected:
        void set_definition(const generic_window_definition& definition);

    private:
        generic_window_definition m_definition;
    };
}