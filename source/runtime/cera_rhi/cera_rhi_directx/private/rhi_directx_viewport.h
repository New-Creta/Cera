#pragma once

#include "util/types.h"

#include "rhi_viewport.h"
#include "rhi_device_child.h"

namespace cera
{
    namespace renderer
    {
        class d3d12_adapter;
        class d3d12_swapchain;

        class d3d12_rhi_viewport : public rhi_viewport, public d3d12_device_child
        {
          public:
            d3d12_rhi_viewport(d3d12_device* in_parent);
            ~d3d12_rhi_viewport();

            void initialize(void* in_window_handle, s32 in_size_x, s32 in_size_y, bool in_is_fullscreen);
            void resize(s32 in_size_x, s32 in_size_y, bool in_is_fullscreen);
            bool present(bool lock_to_vsync);

            void* get_native_swapchain() const override;
            void* get_native_window() const override;

            s32 get_num_back_buffers() const;

            bool is_fullscreen() const;

          private:
            std::unique_ptr<d3d12_swapchain> m_swapchain;
        };
    } // namespace renderer
} // namespace cera