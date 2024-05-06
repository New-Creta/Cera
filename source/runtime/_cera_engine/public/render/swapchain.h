#pragma once

#include "util/types.h"

#include "device/windows_types.h"

#include "render/d3dx12_declarations.h"
#include "render/render_target.h"

namespace cera
{
    class device;
    class texture;
    class command_queue;

    class swapchain
    {
    public:
        static constexpr u32 get_num_back_buffers()
        {
            return s_buffer_count;
        }

    public:
        /**
         * Check to see if the swap chain is in full-screen exclusive mode.
         */
        bool is_fullscreen() const;

        /**
         * Set the swap chain to fullscreen exclusive mode (true) or windowed mode (false).
         */
        void set_fullscreen(bool fullscreen);

        /**
         * Toggle fullscreen exclusive mode.
         */
        void toggle_fullscreen();

        void set_v_sync(bool vSync);

        bool get_v_sync() const;

        void toggle_v_sync();

        /**
         * Check to see if tearing is supported.
         *
         * @see https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/variable-refresh-rate-displays
         */
        bool is_tearing_supported() const;

        /**
         * Block the current thread until the swapchain has finished presenting.
         * Doing this at the beginning of the update loop can improve input latency.
         */
        void wait_for_swapchain();

        /**
         * Resize the swapchain's back buffers. This should be called whenever the window is resized.
         */
        bool on_resize(s32 clientWidth, s32 clientHeight);

        /**
         * Get the render target of the window. This method should be called every
         * frame since the color attachment point changes depending on the window's
         * current back buffer.
         */
        const render_target& get_render_target() const;

        /**
         * Present the swapchain's back buffer to the screen.
         *
         * @param [texture] The texture to copy to the swap chain's backbuffer before
         * presenting. By default, this is an empty texture. In this case, no copy
         * will be performed. Use the SwapChain::GetRenderTarget method to get a render
         * target for the window's color buffer.
         *
         * @returns The current backbuffer index after the present.
         */
        u32 present(const std::shared_ptr<texture>& texture = nullptr);

        /**
         * Get the format that is used to create the backbuffer.
         */
        DXGI_FORMAT get_render_target_format() const;

        Microsoft::WRL::ComPtr<IDXGISwapChain4> get_dxgi_swap_chain() const;

    protected:
        // Swap chains can only be created through the Device.
        swapchain(device& device, HWND hWnd, DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R10G10B10A2_UNORM);
        virtual ~swapchain();

        bool update_render_target_views();

    private:
        // Number of swapchain back buffers.
        static constexpr u32 s_buffer_count = 3;

    private:
        // The device that was used to create the SwapChain.
        device& m_device;

        // The command queue that is used to create the swapchain.
        // The command queue will be signaled right after the Present
        // to ensure that the swapchain's back buffers are not in-flight before
        // the next frame is allowed to be rendered.
        command_queue& m_command_queue;

        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_dxgi_swap_chain;
        std::shared_ptr<texture> m_back_buffer_textures[s_buffer_count];
        mutable render_target m_render_target;

        // The current backbuffer index of the swap chain.
        UINT   m_current_back_buffer_index;
        UINT64 m_fence_values[s_buffer_count];  // The fence values to wait for before leaving the Present method.

        // A handle to a waitable object. Used to wait for the swapchain before presenting.
        HANDLE m_h_frame_latency_waitable_object;

        // The window handle that is associates with this swap chain.
        HWND m_hwnd;

        // The current width/height of the swap chain.
        uint32_t m_width;
        uint32_t m_height;

        // The format of the back buffer.
        DXGI_FORMAT m_render_target_format;

        // Should presentation be synchronized with the vertical refresh rate of the screen?
        // Set to true to eliminate screen tearing.
        bool m_v_sync;

        // Whether or not tearing is supported.
        bool m_tearing_supported;

        // Whether the application is in full-screen exclusive mode or windowed mode.
        bool m_fullscreen;
    };
}