#include "rhi_directx_swapchain.h"
#include "command_list.h"
#include "command_queue.h"
#include "resources/resource_state_tracker.h"
#include "resources/texture.h"
#include "rhi_directx_call.h"
#include "rhi_directx_device.h"
#include "util/assert.h"
#include "util/log.h"

namespace cera
{
    namespace renderer
    {
        d3d12_swapchain::d3d12_swapchain(d3d12_device& device, HWND hwnd, s32 client_width, s32 client_height, DXGI_FORMAT render_target_format)
            : m_device(device)
            , m_command_queue(device.command_queue(D3D12_COMMAND_LIST_TYPE_DIRECT))
            , m_hwnd(hwnd)
            , m_fence_values{0}
            , m_width(client_width)
            , m_height(client_height)
            , m_render_target_format(render_target_format)
            , m_v_sync(true)
            , m_tearing_supported(false)
            , m_fullscreen(false)
        {
            CERA_ASSERT_X(hwnd, "Invalid window handle given"); // Must be a valid window handle!

            // Query the direct command queue from the device.
            // This is required to create the d3d12_swapchain.
            auto d3d_command_queue = m_command_queue.d3d_command_queue();

            // Query the factory from the dxgi_adapter that was used to create the device.
            auto dxgi_adapter = m_device.dxgi_adapter();

            // Get the factory that was used to create the dxgi_adapter.
            wrl::com_ptr<IDXGIFactory> dxgi_factory;
            wrl::com_ptr<IDXGIFactory5> dxgi_factory5;

            HRESULT hr = S_OK;

            hr = dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory));
            CERA_ASSERT_X(SUCCEEDED(hr), "Unable to query IDXGIFactory");

            // Now get the DXGIFactory5 so I can use the IDXGIFactory5::CheckFeatureSupport method.
            hr = dxgi_factory.As(&dxgi_factory5);
            CERA_ASSERT_X(SUCCEEDED(hr) "Unable to query IDXGIFactory5");

            // Check for tearing support.
            BOOL allow_tearing = FALSE;
            hr = dxgi_factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(BOOL));
            if (SUCCEEDED(hr))
            {
                m_tearing_supported = (allow_tearing == TRUE);
            }

            // Query the windows client width and height.
            RECT window_rect;
            ::GetClientRect(hwnd, &window_rect);

            // m_width  = window_rect.right - window_rect.left;
            // m_height = window_rect.bottom - window_rect.top;

            DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
            swap_chain_desc.Width = m_width;
            swap_chain_desc.Height = m_height;
            swap_chain_desc.Format = m_render_target_format;
            swap_chain_desc.Stereo = FALSE;
            swap_chain_desc.SampleDesc = {1, 0};
            swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
            swap_chain_desc.BufferCount = s_buffer_count;
            swap_chain_desc.Scaling = DXGI_SCALING_NONE;
            swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            // It is recommended to always allow tearing if tearing support is available.
            swap_chain_desc.Flags = m_tearing_supported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
            swap_chain_desc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

            // Now create the swap chain.
            wrl::com_ptr<IDXGISwapChain1> dxgi_swap_chain1;
            hr = dxgi_factory5->CreateSwapChainForHwnd(d3d_command_queue.Get(), m_hwnd, &swap_chain_desc, nullptr, nullptr, &dxgi_swap_chain1);

            if (DX_FAILED(hr))
            {
                log::error("Failed to create swapchain with the following parameters:");
                log::error("\tDXGI_MODE_DESC: width: {} height: {} DXGI format: {}", swap_chain_desc.Width, swap_chain_desc.Height, (s32)swap_chain_desc.Format);
                log::error("\tBack buffer count: {}", num_back_buffers());
                log::error("\tSwapchain flags: {}", (s32)swap_chain_desc.Flags);

                CERA_ASSERT("Execution halted due to swapchain creation failure");
            }

            // Cast to swapchain4
            hr = dxgi_swap_chain1.As(&m_dxgi_swap_chain);
            CERA_ASSERT_X(SUCCEEDED(hr), "Unable to querry IDXGISwapchain4");

            // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
            // will be handled manually.
            hr = dxgi_factory5->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
            CERA_ASSERT_X(SUCCEEDED(hr), "Unable to make a window association for IDXGISwapchain4");

            // Initialize the current back buffer index.
            m_current_back_buffer_index = m_dxgi_swap_chain->GetCurrentBackBufferIndex();

            // Set maximum frame latency to reduce input latency.
            m_dxgi_swap_chain->SetMaximumFrameLatency(s_buffer_count - 1);
            // Get the SwapChain's waitable object.
            m_h_frame_latency_waitable_object = m_dxgi_swap_chain->GetFrameLatencyWaitableObject();

            update_render_target_views();
        }

        d3d12_swapchain::~d3d12_swapchain() = default;

        bool d3d12_swapchain::is_fullscreen() const
        {
            return m_fullscreen;
        }

        void d3d12_swapchain::set_fullscreen(bool fullscreen)
        {
            if (m_fullscreen != fullscreen)
            {
                m_fullscreen = fullscreen;
            }
        }

        void d3d12_swapchain::toggle_fullscreen()
        {
            set_fullscreen(!m_fullscreen);
        }

        void d3d12_swapchain::set_v_sync(bool vSync)
        {
            m_v_sync = vSync;
        }

        bool d3d12_swapchain::v_sync() const
        {
            return m_v_sync;
        }

        void d3d12_swapchain::toggle_v_sync()
        {
            set_v_sync(!m_v_sync);
        }

        bool d3d12_swapchain::is_tearing_supported() const
        {
            return m_tearing_supported;
        }

        void d3d12_swapchain::wait_for_swapchain()
        {
            ::WaitForSingleObjectEx(m_h_frame_latency_waitable_object, 1000, TRUE); // Wait for 1 second (should never have to wait that long...)
        }

        bool d3d12_swapchain::on_resize(s32 clientWidth, s32 clientHeight)
        {
            if (m_width != clientWidth || m_height != clientHeight)
            {
                m_width = (std::max)(1, clientWidth);
                m_height = (std::max)(1, clientHeight);

                m_device.flush();

                // Release all references to back buffer textures.
                m_render_target.reset();

                for (u32 i = 0; i < s_buffer_count; ++i)
                {
                    m_back_buffer_textures[i].reset();
                }

                DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
                if (DX_FAILED(m_dxgi_swap_chain->GetDesc(&swap_chain_desc)))
                {
                    log::error("Failed to retrieve d3d12_swapchain description");
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

        RenderTarget& d3d12_swapchain::render_target()
        {
            m_render_target.attach_texture(AttachmentPoint::Color0, m_back_buffer_textures[m_current_back_buffer_index]);
            return m_render_target;
        }

        const RenderTarget& d3d12_swapchain::render_target() const
        {
            m_render_target.attach_texture(AttachmentPoint::Color0, m_back_buffer_textures[m_current_back_buffer_index]);
            return m_render_target;
        }

        u32 d3d12_swapchain::present(const std::shared_ptr<Texture>& texture)
        {
            auto command_list = m_command_queue.command_list();
            auto back_buffer = m_back_buffer_textures[m_current_back_buffer_index];

            if (texture)
            {
                if (texture->d3d_resource_desc().SampleDesc.Count > 1)
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

            u32 sync_interval = m_v_sync ? 1 : 0;
            u32 present_flags = m_tearing_supported && !m_fullscreen && !m_v_sync ? DXGI_PRESENT_ALLOW_TEARING : 0;
            if (DX_FAILED(m_dxgi_swap_chain->Present(sync_interval, present_flags)))
            {
                log::error("Unable to present!");
                return static_cast<u32>(-1);
            }

            m_fence_values[m_current_back_buffer_index] = m_command_queue.signal();

            m_current_back_buffer_index = m_dxgi_swap_chain->GetCurrentBackBufferIndex();

            auto fence_value = m_fence_values[m_current_back_buffer_index];
            m_command_queue.wait_for_fence_value(fence_value);

            m_device.release_stale_descriptors();

            return m_current_back_buffer_index;
        }

        DXGI_FORMAT d3d12_swapchain::render_target_format() const
        {
            return m_render_target_format;
        }

        wrl::com_ptr<IDXGISwapChain4> d3d12_swapchain::dxgi_swap_chain() const
        {
            return m_dxgi_swap_chain;
        }

        HWND d3d12_swapchain::hwnd() const
        {
            return m_hwnd;
        }

        bool d3d12_swapchain::update_render_target_views()
        {
            for (u32 i = 0; i < s_buffer_count; ++i)
            {
                wrl::com_ptr<ID3D12Resource> back_buffer;
                if (DX_FAILED(m_dxgi_swap_chain->GetBuffer(i, IID_PPV_ARGS(&back_buffer))))
                {
                    log::error("Unable to retrieve d3d12_swapchain buffer");
                    return false;
                }

                ResourceStateTracker::add_global_resource_state(back_buffer.Get(), D3D12_RESOURCE_STATE_COMMON);

                m_back_buffer_textures[i] = m_device.create_texture(back_buffer);

                // Set the names for the backbuffer textures.
                // Useful for debugging.
                m_back_buffer_textures[i]->set_resource_name(L"Backbuffer[" + std::to_wstring(i) + L"]");
            }

            return true;
        }
    } // namespace renderer
} // namespace cera