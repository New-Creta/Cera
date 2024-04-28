#pragma once

#include "util/types.h"

#include "generic_application.h"

#include "windows/win32_min.h"

#include <vector>

namespace cera
{
    class generic_application_message_handler;
    class generic_window_definition;
    class generic_window;

    /**
     * Interface for classes that handle Windows events.
     */
    class iwindows_message_handler
    {
    public:
        /**
         * Processes a Windows message.
         *
         * @param hwnd Handle to the window that received the message.
         * @param msg The message.
         * @param wparam Additional message information.
         * @param lparam Additional message information.
         * @param OutResult Will contain the result if the message was handled.
         * @return true if the message was handled, false otherwise.
         */
        virtual bool process_message(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam, s32& out_result) = 0;
    };

    struct deferred_windows_message
    {
        deferred_windows_message(const std::shared_ptr<class windows_window> &in_native_window, HWND in_hwnd, u32 in_message, WPARAM in_wparam, LPARAM in_lparam, s32 in_x = 0, s32 in_y = 0, u32 in_raw_input_flags = 0)
            : native_window(in_native_window), hwnd(in_hwnd), message(in_message), wparam(in_wparam), lparam(in_lparam), x(in_x), y(in_y), raw_input_flags(in_raw_input_flags)
        {
        }

        /** Native window that received the message */
        std::weak_ptr<class windows_window> native_window;

        /** Window handle */
        HWND hwnd;

        /** message code */
        u32 message;

        /** message data */
        WPARAM wparam;
        LPARAM lparam;

        /** Mouse coordinates */
        s32 x;
        s32 y;
        u32 raw_input_flags;
    };

    class windows_application : public generic_application
    {
    public:
        /**
         * Static: Creates a new Win32 application
         *
         * @param instance_handle Win32 instance handle.
         * @param icon_handle Win32 application icon handle.
         * @return New application object.
         */
        static std::shared_ptr<windows_application> create_windows_application(const HINSTANCE instance_handle, const HICON icon_handle);

        /** constructor. */
        windows_application(const HINSTANCE hinstance, const HICON hicon);

        /** Virtual destructor. */
        virtual ~windows_application();

        /**
         * Adds a Windows message handler with the application instance.
         *
         * @param in_message_handler The message handler to register.
         * @see RemoveMessageHandler
         */
        virtual void add_message_handler(const std::shared_ptr<iwindows_message_handler>& in_message_handler);

        /**
         * Removes a Windows message handler with the application instance.
         *
         * @param in_message_handler The message handler to register.
         * @see AddMessageHandler
         */
        void remove_message_handler(const std::shared_ptr<iwindows_message_handler>& in_message_handler);

    public:
        // generic window API
        void process_deferred_events() override;
        std::shared_ptr<generic_window> make_window() override;
        void initialize_window(const std::shared_ptr<generic_window>& in_window, const generic_window_definition& definition, const bool show) override;
        void destroy_application() override;

    protected:
	    friend LRESULT windows_application_wnd_proc(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam);

	    /** Windows callback for message processing (forwards messages to the FWindowsApplication instance). */
	    static LRESULT CALLBACK app_wnd_proc(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam);

	    /** Processes a single Windows message. */
	    s32 process_message( HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam );

        /** Processes a deferred Windows message. */
	    s32 process_deferred_message( const deferred_windows_message& deferred_message );

    private:
        static const POINT s_minimized_window_position;

        /** Registers the Windows class for windows and assigns the application instance and icon */
        static bool register_class(const HINSTANCE hinstance, const HICON hicon);

	    /**  @return  True if a windows message is related to user input from the keyboard */
	    static bool is_keyboard_input_message( u32 msg );

	    /**  @return  True if a windows message is related to user input from the mouse */
	    static bool is_mouse_input_message( u32 msg );

        /**  @return  True if a windows message is related to user input (mouse, keyboard) */
	    static bool is_input_message( u32 msg );

	    /** Defers a Windows message for later processing. */
	    void defer_message( std::shared_ptr<class windows_window>& native_window, HWND in_hwnd, u32 in_message, WPARAM in_wparam, LPARAM in_lparam, s32 mouse_x = 0, s32 mouse_y = 0, u32 raw_input_flags = 0 );

        /** Checks a key code for release of the Shift key. */
	    void check_for_shift_up_events(const s32 key_code);

        /** Helper function to update the cached states of all modifier keys */
	    void update_all_modifier_key_states();

    private:
        enum modifier_key
        {
            left_shift,		// VK_LSHIFT
            right_shift,	// VK_RSHIFT
            left_control,	// VK_LCONTROL
            right_control,	// VK_RCONTROL
            left_alt,		// VK_LMENU
            right_alt,		// VK_RMENU
            caps_lock,		// VK_CAPITAL
            count,
        };

    private:
        HINSTANCE m_instance_handle;

        bool m_consome_alt_space;

        bool m_in_modal_size_loop;

        bool m_allowed_to_defer_message_processing;

        bool m_force_activate_by_mouse;

        std::vector<deferred_windows_message> m_deferred_messages;

	    std::vector<std::shared_ptr<iwindows_message_handler>> m_message_handlers;

	    std::vector<std::shared_ptr<class windows_window>> m_windows;

        /** Cached state of the modifier keys. True if the modifier key is pressed (or toggled in the case of caps lock), false otherwise */
	    bool m_modifier_key_state[modifier_key::count];
    };
}