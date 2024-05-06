#include "render/swapchain.h"
#include "render/command_queue.h"
#include "render/command_list.h"
#include "render/resource_state_tracker.h"
#include "render/d3dx12_call.h"
#include "render/device.h"
#include "render/texture.h"

#include "device/windows_declarations.h"

#include "util/log.h"

namespace cera
{
    swapchain::swapchain(device& device, HWND hwnd, DXGI_FORMAT renderTargetFormat)
        : m_device(device)
        , m_command_queue(device.get_command_queue(D3D12_COMMAND_LIST_TYPE_DIRECT))
        , m_hwnd(hwnd)
        , m_fence_values{ 0 }
        , m_width(0u)
        , m_height(0u)
        , m_render_target_format(renderTargetFormat)
        , m_v_sync(true)
        , m_tearing_supported(false)
        , m_fullscreen(false)
    {
        assert(hwnd);  // Must be a valid window handle!

        // Query the direct command queue from the device.
        // This is required to create the swapchain.
        auto d3d_command_queue = m_command_queue.get_d3d_command_queue();

        // Query the factory from the dxgi_adapter that was used to create the device.
        auto dxgi_adapter = m_device.get_dxgi_adapter();

        // Get the factory that was used to create the dxgi_adapter.
        Microsoft::WRL::ComPtr<IDXGIFactory>  dxgi_factory;
        Microsoft::WRL::ComPtr<IDXGIFactory5> dxgi_factory5;

        HRESULT hr = S_OK;

        hr = dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory));
        assert(SUCCEEDED(hr));

        // Now get the DXGIFactory5 so I can use the IDXGIFactory5::CheckFeatureSupport method.
        hr = dxgi_factory.As(&dxgi_factory5);
        assert(SUCCEEDED(hr));

        // Check for tearing support.
        BOOL allow_tearing = FALSE;
        hr = dxgi_factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(BOOL));
        if(SUCCEEDED(hr))
        {
            m_tearing_supported = (allow_tearing == TRUE);
        }

        // Query the windows client width and height.
        RECT window_rect;
        ::GetClientRect(hwnd, &window_rect);

        m_width = window_rect.right - window_rect.left;
        m_height = window_rect.bottom - window_rect.top;

        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
        swap_chain_desc.Width = m_width;
        swap_chain_desc.Height = m_height;
        swap_chain_desc.Format = m_render_target_format;
        swap_chain_desc.Stereo = FALSE;
        swap_chain_desc.SampleDesc = { 1, 0 };
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount = s_buffer_count;
        swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        // It is recommended to always allow tearing if tearing support is available.
        swap_chain_desc.Flags = m_tearing_supported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        swap_chain_desc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

        // Now create the swap chain.
        Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgi_swap_chain1;
        hr = dxgi_factory5->CreateSwapChainForHwnd(d3d_command_queue.Get(), m_hwnd, &swap_chain_desc, nullptr, nullptr, &dxgi_swap_chain1);
        assert(SUCCEEDED(hr));

        // Cast to swapchain4
        hr = dxgi_swap_chain1.As(&m_dxgi_swap_chain);
        assert(SUCCEEDED(hr));

        // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
        // will be handled manually.
        hr = dxgi_factory5->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
        assert(SUCCEEDED(hr));

        // Initialize the current back buffer index.
        m_current_back_buffer_index = m_dxgi_swap_chain->GetCurrentBackBufferIndex();

        // Set maximum frame latency to reduce input latency.
        m_dxgi_swap_chain->SetMaximumFrameLatency( s_buffer_count - 1 );
        // Get the SwapChain's waitable object.
        m_h_frame_latency_waitable_object = m_dxgi_swap_chain->GetFrameLatencyWaitableObject();

        update_render_target_views();
    }

    swapchain::~swapchain() = default;

    bool swapchain::is_fullscreen() const
    {
        return m_fullscreen;
    }

    void swapchain::set_fullscreen(bool fullscreen)
    {
        if (m_fullscreen != fullscreen)
        {
            m_fullscreen = fullscreen;
        }
    }

    void swapchain::toggle_fullscreen()
    {
        set_fullscreen(!m_fullscreen);
    }

    void swapchain::set_v_sync(bool vSync)
    {
        m_v_sync = vSync;
    }

    bool swapchain::get_v_sync() const
    {
        return m_v_sync;
    }

    void swapchain::toggle_v_sync()
    {
        set_v_sync(!m_v_sync);
    }

    bool swapchain::is_tearing_supported() const
    {
        return m_tearing_supported;
    }

    void swapchain::wait_for_swapchain()
    {
        ::WaitForSingleObjectEx(m_h_frame_latency_waitable_object, 1000, TRUE);  // Wait for 1 second (should never have to wait that long...)
    }

    bool swapchain::on_resize(s32 clientWidth, s32 clientHeight)
    {
        if (m_width != clientWidth || m_height != clientHeight)
        {
            m_width = std::max(1, clientWidth);
            m_height = std::max(1, clientHeight);

            m_device.flush();

            // Release all references to back buffer textures.
            m_render_target.reset();

            for (UINT i = 0; i < s_buffer_count; ++i)
            {
                m_back_buffer_textures[i].reset();
            }

            DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
            if (DX_FAILED(m_dxgi_swap_chain->GetDesc(&swap_chain_desc)))
            {
                log::error("Failed to retrieve swapchain description");
                return false;
            }
            
            if (DX_FAILED(m_dxgi_swap_chain->ResizeBuffers(s_buffer_count, m_width, m_height, swap_chain_desc.BufferDesc.Format, swap_chain_desc.Flags)))
            {
                log::error("Failed to resize buffers");
                return false;
            }

            m_current_back_buffer_index = m_dxgi_swap_chain->GetCurrentBackBufferIndex();

            update_render_target_views();
        }

        return true;
    }

    const render_target& swapchain::get_render_target() const
    {
        m_render_target.attach_texture(attachment_point::color_0, m_back_buffer_textures[m_current_back_buffer_index]);
        return m_render_target;
    }

    u32 swapchain::present(const std::shared_ptr<texture>& texture)
    {
        auto command_list = m_command_queue.get_command_list();
        auto back_buffer = m_back_buffer_textures[m_current_back_buffer_index];

        if (texture)
        {
            if (texture->get_d3d_resource_desc().SampleDesc.Count > 1)
            {
                command_list->resolve_subresource(back_buffer, texture);
            }
            else
            {
                command_list->copy_resource(back_buffer, texture);
            }
        }

        command_list->transition_barrier(back_buffer, D3D12_RESOURCE_STATE_PRESENT);

        m_command_queue.execute_command_list(command_list);

        UINT sync_interval = m_v_sync ? 1 : 0;
        UINT present_flags = m_tearing_supported && !m_fullscreen && !m_v_sync ? DXGI_PRESENT_ALLOW_TEARING : 0;
        if (DX_FAILED(m_dxgi_swap_chain->Present(sync_interval, present_flags)))
        {
            log::error("Unable to present!");
            return -1;
        }

        m_fence_values[m_current_back_buffer_index] = m_command_queue.signal();

        m_current_back_buffer_index = m_dxgi_swap_chain->GetCurrentBackBufferIndex();

        auto fence_value = m_fence_values[m_current_back_buffer_index];
        m_command_queue.wait_for_fence_value(fence_value);

        m_device.release_stale_descriptors();

        return m_current_back_buffer_index;
    }

    DXGI_FORMAT swapchain::get_render_target_format() const
    {
        return m_render_target_format;
    }

    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapchain::get_dxgi_swap_chain() const
    {
        return m_dxgi_swap_chain;
    }

    bool swapchain::update_render_target_views()
    {
        for (UINT i = 0; i < s_buffer_count; ++i)
        {
            ComPtr<ID3D12Resource> backBuffer;
            if (DX_FAILED(m_dxgi_swap_chain->GetBuffer(i, IID_PPV_ARGS(&backBuffer))))
            {
                log::error("Unable to retrieve swapchain buffer");
                return false;
            }

            resource_state_tracker::add_global_resource_state(backBuffer.Get(), D3D12_RESOURCE_STATE_COMMON);

            m_back_buffer_textures[i] = m_device.create_texture(backBuffer);

            // Set the names for the backbuffer textures.
            // Useful for debugging.
            m_back_buffer_textures[i]->set_resource_name(L"Backbuffer[" + std::to_wstring(i) + L"]");
        }

        return true;
    }
}