#include "dxgi/objects/adapter.h"
#include "dxgi/dxgi_util.h"

#include "directx_call.h"

#include "rhi_globals.h"

#include "util/log.h"
#include "util/types.h"
#include "util/memory_size.h"
#include "util/string_op.h"

#include "wrl/comobject.h"

#include <cstdlib>
#include <dxgi.h>
#include <string>

namespace cera
{
    namespace dxgi
    {
        //-------------------------------------------------------------------------
        const TCHAR* get_feature_level_string(D3D_FEATURE_LEVEL FeatureLevel)
        {
            switch (FeatureLevel)
            {
            case D3D_FEATURE_LEVEL_9_1:
                return TEXT("9_1");
            case D3D_FEATURE_LEVEL_9_2:
                return TEXT("9_2");
            case D3D_FEATURE_LEVEL_9_3:
                return TEXT("9_3");
            case D3D_FEATURE_LEVEL_10_0:
                return TEXT("10_0");
            case D3D_FEATURE_LEVEL_10_1:
                return TEXT("10_1");
            case D3D_FEATURE_LEVEL_11_0:
                return TEXT("11_0");
            case D3D_FEATURE_LEVEL_11_1:
                return TEXT("11_1");
            case D3D_FEATURE_LEVEL_12_0:
                return TEXT("12_0");
            case D3D_FEATURE_LEVEL_12_1:
                return TEXT("12_1");
#if CERA_D3D12_CORE_ENABLED
            case D3D_FEATURE_LEVEL_12_2:
                return TEXT("12_2");
#endif
            }
            return TEXT("X_X");
        }

        //-------------------------------------------------------------------------
        u32 count_adapter_outputs(cera::wrl::ComPtr<IDXGIAdapter> adapter)
        {
            u32 output_count = 0;
            for (;;)
            {
                cera::wrl::ComPtr<IDXGIOutput> output;
                HRESULT hr = adapter->EnumOutputs(output_count, output.GetAddressOf());
                if (FAILED(hr))
                {
                    break;
                }

                ++output_count;
            }

            return output_count;
        }

        //-------------------------------------------------------------------------
        bool is_adapter_integrated(wrl::ComPtr<IDXGIAdapter> in_adapter)
        {
            wrl::ComPtr<IDXGIAdapter3> adapter3;
            in_adapter->QueryInterface(IID_PPV_ARGS(adapter3.GetAddressOf()));

            // Simple heuristic but without profiling it's hard to do better
            DXGI_QUERY_VIDEO_MEMORY_INFO non_ocal_video_memory_info {};
            if (adapter3 && DX_SUCCESS(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &non_ocal_video_memory_info)))
            {
                return non_ocal_video_memory_info.Budget == 0;
            }

            return true;
        }

        //-------------------------------------------------------------------------
        adapter_description::adapter_description() = default;
        //-------------------------------------------------------------------------
        adapter_description::adapter_description(const DXGI_ADAPTER_DESC& in_desc, s32 in_index, const adapter_device_info& in_device_info, bool in_is_integrated)
            : desc(in_desc)
	        , adapter_index(in_index)
	        , max_supported_feature_level(in_device_info.max_feature_level)
	        , max_supported_shader_model(in_device_info.max_shader_model)
	        , resource_binding_tier(in_device_info.feature_data_options.resource_binding_tier)
            , resource_heap_tier(in_device_info.feature_data_options.resource_heap_tier)
	        , max_rhi_feature_level(in_device_info.max_rhi_feature_level)
	        , supports_wave_ops(in_device_info.supports_wave_ops)
	        , supports_atomic64(in_device_info.supports_atomic64)
            , num_device_nodes(in_device_info.num_device_nodes)
            , is_integrated(in_is_integrated)
        {

        }

        //-------------------------------------------------------------------------
        bool adapter_description::is_valid() const
        {
            return max_supported_feature_level != (D3D_FEATURE_LEVEL)0 && adapter_index >= 0;
        }

        //-------------------------------------------------------------------------
        adapter::adapter(s32 in_adapter_index, const adapter_device_info& in_device_info, wrl::ComPtr<IDXGIAdapter> adapter, const DXGI_ADAPTER_DESC& in_adapter_desc)
            : ComObject(adapter)
        {
            const s32 output_count = count_adapter_outputs(adapter);

            cera::log::info("Found D3D12 adapter {0}: {1} (VendorId: {2}, DeviceId: {3}, SubSysId: {4}, Revision: {5}"
                , in_adapter_index
                , in_adapter_desc.Description
                , in_adapter_desc.VendorId
                , in_adapter_desc.DeviceId
                , in_adapter_desc.SubSysId
                , in_adapter_desc.Revision);

            cera::log::info("  Max supported Feature Level {0}, shader model {1}.{2}, binding tier {3}, wave ops {4}, atomic64 {5}"
                , get_feature_level_string(in_device_info.max_feature_level)
                , (in_device_info.max_shader_model >> 4)
                , (in_device_info.max_shader_model & 0xF)
                , in_device_info.feature_data_options.resource_binding_tier
                , in_device_info.supports_wave_ops 
                    ? TEXT("supported") 
                    : TEXT("unsupported")
                , in_device_info.supports_atomic64 
                    ? TEXT("supported") 
                    : TEXT("unsupported"));

            cera::log::info("  Adapter has {0}MB of dedicated video memory, {1}MB of dedicated system memory, and {2}MB of shared system memory, {3} output[s]"
                , (u32)(in_adapter_desc.DedicatedVideoMemory / (1024 * 1024))
                , (u32)(in_adapter_desc.DedicatedSystemMemory / (1024 * 1024))
                , (u32)(in_adapter_desc.SharedSystemMemory / (1024 * 1024))
                , output_count);

            m_description = cera::dxgi::adapter_description(in_adapter_desc, in_adapter_index, in_device_info, is_adapter_integrated(adapter));
        }

        //-------------------------------------------------------------------------
        const adapter_description& adapter::description() const
        {
            return m_description;
        }
    } // namespace dxgi
} // namespace cera
