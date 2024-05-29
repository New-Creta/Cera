#pragma once

#include "util/types.h"

#include "wrl/comobject.h"

#include "common/rhi_feature_level.h"

#include "rhi_directx_util.h"

#include "dxgi/dxgi_util.h"

namespace cera
{
    namespace renderer
    {
        struct adapter_feature_data_options
        {
            D3D12_RESOURCE_BINDING_TIER resource_binding_tier;
            D3D12_RESOURCE_HEAP_TIER resource_heap_tier;
        };

        struct adapter_device_info
        {
            D3D_FEATURE_LEVEL               max_feature_level;
            D3D_SHADER_MODEL                max_shader_model;
            adapter_feature_data_options    feature_data_options;
            u32                             num_device_nodes;
            bool                            supports_wave_ops;
            bool                            supports_atomic64;

            renderer::feature_level         max_rhi_feature_level;
        };

        struct adapter_description
        {
            adapter_description();
            adapter_description(const DXGI_ADAPTER_DESC& in_desc, s32 in_index, const adapter_device_info& in_device_info, bool is_integrated);

            bool is_valid() const;

            /* Adapter description */
            DXGI_ADAPTER_DESC desc = {};

            /** -1 if not supported or FindAdapter() wasn't called. Ideally we would store a pointer to IDXGIAdapter but it's unlikely the adpaters change during engine init. */
            s32 adapter_index = -1;

            /** The maximum D3D12 feature level supported. 0 if not supported or FindAdapter() wasn't called */
            D3D_FEATURE_LEVEL max_supported_feature_level = (D3D_FEATURE_LEVEL)0;

            /** The maximum Shader Model supported. 0 if not supported or FindAdpater() wasn't called */
            D3D_SHADER_MODEL max_supported_shader_model = (D3D_SHADER_MODEL)0;

            /* Identifies the tier of resource binding being used. */
            D3D12_RESOURCE_BINDING_TIER resource_binding_tier = D3D12_RESOURCE_BINDING_TIER_1;

            /* Specifies which resource heap tier the hardware and driver support. */
            D3D12_RESOURCE_HEAP_TIER resource_heap_tier = D3D12_RESOURCE_HEAP_TIER_1;

            /* Feature level of the adapter */
            renderer::feature_level max_rhi_feature_level = renderer::feature_level::NUM;

            /** number of device nodes (read: gp_us) */
            u32 num_device_nodes = 1;

            /** whether the gpu is integrated or discrete. */
            bool is_integrated = false;

            /** whether sm6.0 wave ops are supported */
            bool supports_wave_ops = false;

            /** whether sm6.6 atomic64 wave ops are supported */
            bool supports_atomic64 = false;
        };

        class adapter : public wrl::ComObject<IDXGIAdapter> // NOLINT(fuchsia-multiple-inheritance)
        {
          public:
            adapter(s32 in_adapter_index, const adapter_device_info& in_device_info, wrl::com_ptr<IDXGIAdapter> adapter, const DXGI_ADAPTER_DESC& in_adapter_desc);

            const adapter_description& description() const;

          private:
            adapter_description m_description;
        };
    } // namespace renderer
} // namespace cera