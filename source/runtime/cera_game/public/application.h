#pragma once

#include "device/windows_types.h"

#include <string>
#include <memory>

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace cera
{
    class abstract_game;

    class device;
    class window;
    class swapchain;
    class gui;
    class render_target;

    class application
    {
    public:
        static bool create(win::HInstance hinstance);
        static void destroy();

        static application* get();

        static u64 get_frame_count();

    public:
        application(application& other) = delete;
        void operator=(const application&) = delete;

    public:
        ~application();

        s32 run(abstract_game* game, s32 clientWidth, s32 clientHeight, const std::wstring& wndTitle);

        void quit();

        const std::shared_ptr<device>& get_device() const;
        const std::shared_ptr<window>& get_window() const;
        const std::shared_ptr<swapchain>& get_swapchain() const;
        const std::shared_ptr<gui> get_gui() const;

    private:
        application(win::HInstance hinstance);

    private:
        /**
        * Make std::make_unique a friend of in order to allow it to access private constructors.
        */
        template <class _Type, class... _Types, std::enable_if_t<!std::is_array_v<_Type>, int>>
        friend std::unique_ptr<_Type> std::make_unique<_Type>(_Types&&...);

        /*
        * Make WndProc a friend so we are able to alter the state of the application from the WndProc
        */
        friend LRESULT CALLBACK ::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        static std::unique_ptr<application> s_instance;
        static u64 s_frame_counter;

    private:
        bool initialize(abstract_game* game, s32 clientWidth, s32 clientHeight, const std::wstring& wndTitle);
        void terminate(abstract_game* game);

        s32 loop(abstract_game* game);

        win::HInstance m_hinstance;

        bool m_request_quit;

        std::shared_ptr<device> m_device;
        std::shared_ptr<window> m_window;
        std::shared_ptr<swapchain> m_swapchain;
        std::shared_ptr<gui> m_gui;
    };
}