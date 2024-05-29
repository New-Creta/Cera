#include "rhi_directx_viewport.h"
#include "rhi_directx_swapchain.h"

namespace cera
{
    namespace renderer
    {
        d3d12_rhi_viewport::d3d12_rhi_viewport(d3d12_device* in_parent) 
            : d3d12_device_child(in_parent)
        {

        }
        
        d3d12_rhi_viewport::~d3d12_rhi_viewport()
        {}
        
        void d3d12_rhi_viewport::initialize(void* in_window_handle, s32 in_size_x, s32 in_size_y, bool in_is_fullscreen)
        {
            m_swapchain = std::make_unique<d3d12_swapchain>(*get_parent_device(), in_window_handle, in_size_x, in_size_y);
            m_swapchain->set_fullscreen(in_is_fullscreen);
        }
        
        void d3d12_rhi_viewport::resize(s32 in_size_x, s32 in_size_y, bool in_is_fullscreen)
        {
            m_swapchain->on_resize(in_size_x, in_size_y);
            m_swapchain->set_fullscreen(in_is_fullscreen);
        }
        
        bool d3d12_rhi_viewport::present(bool lock_to_vsync)
        {
            m_swapchain->set_v_sync(lock_to_vsync);
            m_swapchain->present();
        }
        
        void* d3d12_rhi_viewport::get_native_swapchain() const
        {
            return m_swapchain->dxgi_swap_chain().Get();
        }
        
        void* d3d12_rhi_viewport::get_native_window() const
        {
            return m_swapchain->hwnd();
        }
        
        s32 d3d12_rhi_viewport::get_num_back_buffers() const
        {
            return d3d12_swapchain::num_back_buffers();
        }
        
        bool d3d12_rhi_viewport::is_fullscreen() const
        {
            return m_swapchain->is_fullscreen();
        }
    }
}