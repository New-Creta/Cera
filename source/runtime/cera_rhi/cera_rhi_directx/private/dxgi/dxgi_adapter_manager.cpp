#include "dxgi/dxgi_adapter_manager.h"
#include "dxgi/dxgi_util.h"
#include "dxgi/dxgi_adapter.h" // IWYU pragma: keep

#include "util/assert.h"

#include "directx_call.h"
#include "directx_util.h"

#include "rhi_globals.h"

#include "common/rhi_data_driven_shader_platform_info.h"
#include "common/rhi_feature_level.h"
#include "common/rhi_shader_platform.h"

#include <functional>
#include <optional>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace cera
{
    namespace renderer
    {
        //-------------------------------------------------------------------------
        adapter_feature_data_options get_resource_tiers(ID3D12Device* in_device)
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS d3d12_caps{};

            in_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12_caps, sizeof(d3d12_caps));

            return {d3d12_caps.ResourceBindingTier, d3d12_caps.ResourceHeapTier};
        }
        //-------------------------------------------------------------------------
        bool get_supports_wave_ops(ID3D12Device* in_device)
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS1 d3d12_caps1{};
            in_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &d3d12_caps1, sizeof(d3d12_caps1));

            return d3d12_caps1.WaveOps;
        }
        //-------------------------------------------------------------------------
        bool get_supports_atomic64(IDXGIAdapter* in_adapter, ID3D12Device* in_device)
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS9 d3d12_caps9{};
            in_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS9, &d3d12_caps9, sizeof(d3d12_caps9));

            return d3d12_caps9.AtomicInt64OnTypedResourceSupported;
        }
        //-------------------------------------------------------------------------
        std::function<HRESULT(UINT, wrl::com_ptr<IDXGIAdapter>*)> get_enumaration_function_factory2(wrl::com_ptr<IDXGIFactory2> in_factory)
        {
            CERA_ASSERT_X(in_factory, "IDXGIFactory2 does not exist!");

            return [factory = in_factory.Get()](UINT index, wrl::com_ptr<IDXGIAdapter>* adapter) { return factory->EnumAdapters(index, (*adapter).GetAddressOf()); };
        }
        //-------------------------------------------------------------------------
        std::function<HRESULT(UINT, wrl::com_ptr<IDXGIAdapter>*)> get_enumaration_function_factory6(wrl::com_ptr<IDXGIFactory6> in_factory, DXGI_GPU_PREFERENCE gpu_preference)
        {
            CERA_ASSERT_X(gpu_preference == DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE || gpu_preference == DXGI_GPU_PREFERENCE_MINIMUM_POWER || gpu_preference == DXGI_GPU_PREFERENCE_UNSPECIFIED, "DXGI_GPU_PREFERENCE has an invalid value");
            CERA_ASSERT_X(in_factory, "IDXGIFactory6 does not exist!");

            return [factory = in_factory.Get(), gpu_preference](UINT index, wrl::com_ptr<IDXGIAdapter>* adapter) { return factory->EnumAdapterByGpuPreference(index, gpu_preference, IID_PPV_ARGS((*adapter).GetAddressOf())); };
        }

        //-------------------------------------------------------------------------
        feature_level find_max_rhi_feature_level(D3D_FEATURE_LEVEL in_max_feature_level, D3D_SHADER_MODEL in_max_shader_model, D3D12_RESOURCE_BINDING_TIER resource_binding_tier, bool supports_wave_ops, bool supports_atomic64)
        {
            feature_level max_rhi_feature_level = feature_level::NUM;

            if (in_max_feature_level >= D3D_FEATURE_LEVEL_12_0 && in_max_shader_model >= D3D_SHADER_MODEL_6_6)
            {
                bool high_enough_binding_tier = true;

                if (data_driven_shader_platform_info::is_supports_bindless(shader_platform::D3D_SM6))
                {
                    high_enough_binding_tier = resource_binding_tier >= D3D12_RESOURCE_BINDING_TIER_3;
                }
                else
                {
                    high_enough_binding_tier = resource_binding_tier >= D3D12_RESOURCE_BINDING_TIER_2;
                }

                if (supports_wave_ops && high_enough_binding_tier && supports_atomic64)
                {
                    if (g_force_disable_sm6)
                    {
                        log::info("feature_level::D3D_SM6 disabled");
                    }
                    else
                    {
                        max_rhi_feature_level = feature_level::D3D_SM6;
                    }
                }
            }

            if (max_rhi_feature_level == feature_level::NUM && in_max_feature_level >= D3D_FEATURE_LEVEL_11_0)
            {
                max_rhi_feature_level = feature_level::D3D_SM5;
            }

            return max_rhi_feature_level;
        }
        //-------------------------------------------------------------------------
        D3D_FEATURE_LEVEL find_highest_feature_level(ID3D12Device* in_device, D3D_FEATURE_LEVEL in_min_feature_level)
        {
            const D3D_FEATURE_LEVEL feature_levels_to_check[] = {
#if CERA_D3D12_CORE_ENABLED
                D3D_FEATURE_LEVEL_12_2,
#endif
                D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

            // Determine the max feature level supported by the driver and hardware.
            D3D12_FEATURE_DATA_FEATURE_LEVELS feature_level_caps{};
            feature_level_caps.pFeatureLevelsRequested = feature_levels_to_check;
            feature_level_caps.NumFeatureLevels = ARRAYSIZE(feature_levels_to_check);

            if (DX_SUCCESS(in_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feature_level_caps, sizeof(feature_level_caps))))
            {
                return feature_level_caps.MaxSupportedFeatureLevel;
            }

            return in_min_feature_level;
        }
        //-------------------------------------------------------------------------
        D3D_SHADER_MODEL find_highest_shader_model(ID3D12Device* in_device)
        {
            // Because we can't guarantee older Windows versions will know about newer shader models, we need to check them all
            // in descending order and return the first result that succeeds.
            const D3D_SHADER_MODEL shader_models_to_check[] = {
#if CERA_D3D12_CORE_ENABLED
                D3D_SHADER_MODEL_6_7, D3D_SHADER_MODEL_6_6,
#endif
                D3D_SHADER_MODEL_6_5, D3D_SHADER_MODEL_6_4, D3D_SHADER_MODEL_6_3, D3D_SHADER_MODEL_6_2, D3D_SHADER_MODEL_6_1, D3D_SHADER_MODEL_6_0,
            };

            D3D12_FEATURE_DATA_SHADER_MODEL feature_shader_model{};
            for (const D3D_SHADER_MODEL ShaderModelToCheck : shader_models_to_check)
            {
                feature_shader_model.HighestShaderModel = ShaderModelToCheck;

                if (DX_SUCCESS(in_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &feature_shader_model, sizeof(feature_shader_model))))
                {
                    return feature_shader_model.HighestShaderModel;
                }
            }

            // Last ditch effort, the minimum requirement for DX12 is 5.1
            return D3D_SHADER_MODEL_5_1;
        }

        //-------------------------------------------------------------------------
        /**
         * Attempts to create a D3D12 device for the adapter using at minimum MinFeatureLevel.
         * If creation is successful, true is returned and the max supported feature level is set in OutMaxFeatureLevel.
         */
        std::optional<adapter_device_info> safe_test_d3d12_create_device(wrl::com_ptr<IDXGIAdapter> in_adapter, D3D_FEATURE_LEVEL in_min_feature_level)
        {
            ID3D12Device* d3d12_device = nullptr;
            if (DX_SUCCESS(D3D12CreateDevice(in_adapter.Get(), in_min_feature_level, IID_PPV_ARGS(&d3d12_device))))
            {
                adapter_device_info result;

                result.max_feature_level = find_highest_feature_level(d3d12_device, in_min_feature_level);
                result.max_shader_model = find_highest_shader_model(d3d12_device);
                result.feature_data_options = get_resource_tiers(d3d12_device);
                result.num_device_nodes = d3d12_device->GetNodeCount();
                result.supports_wave_ops = get_supports_wave_ops(d3d12_device);
                result.supports_atomic64 = get_supports_atomic64(in_adapter.Get(), d3d12_device);
                result.max_rhi_feature_level = find_max_rhi_feature_level(result.max_feature_level, result.max_shader_model, result.feature_data_options.resource_binding_tier, result.supports_wave_ops, result.supports_atomic64);

                d3d12_device->Release();

                return result;
            }

            return {};
        }

        //-------------------------------------------------------------------------
        std::vector<std::shared_ptr<adapter>> get_adapters(const std::function<HRESULT(UINT, wrl::com_ptr<IDXGIAdapter>*)>& enumarationFnc)
        {
            const D3D_FEATURE_LEVEL min_require_feature_level = D3D_FEATURE_LEVEL_11_0;

            u32 i = 0;
            wrl::com_ptr<IDXGIAdapter> dxgi_adapter = nullptr;

            std::vector<std::shared_ptr<adapter>> adapters;
            while (enumarationFnc(i, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND)
            {
                if (dxgi_adapter)
                {
                    if (auto device_info = safe_test_d3d12_create_device(dxgi_adapter, min_require_feature_level))
                    {
                        CERA_ASSERT_X(device_info->num_device_nodes > 0, "The number of physical adapters should at least be greater than 0");

                        DXGI_ADAPTER_DESC dxgi_adapter_desc;
                        if (DX_SUCCESS(dxgi_adapter->GetDesc(&dxgi_adapter_desc)))
                        {
                            vendor vendor = conversions::to_gpu_vendor_id(dxgi_adapter_desc.VendorId);

                            bool is_warp = vendor == vendor::MICROSOFT;
                            bool skip_warp = (g_use_warp_adapter && !is_warp) || (!g_use_warp_adapter && is_warp && !g_allow_software_rendering);
                            if (!skip_warp)
                            {
                                adapters.emplace_back(std::make_shared<adapter>(i, device_info, dxgi_adapter, dxgi_adapter_desc));
                            }
                        }
                    }
                }

                ++i;
            }

            return adapters;
        }

        // Declared in rhi_globals.h
        gpu_info g_gpu_info = {};

        //-------------------------------------------------------------------------
        adapter_manager::adapter_manager(const adapter_scorer_fn& scorer, s32 gpu_preference) : m_selected_adapter(nullptr)
        {
            load_adapters(gpu_preference);

            CERA_ASSERT_X(!m_adapters.empty(), "No adapters found");

            // this can be fixed once we have vector views/ranges
            std::vector<adapter_description> gpus;
            gpus.reserve(m_adapters.size());
            for (const auto& adapter : m_adapters)
            {
                gpus.push_back(adapter->description());
            }

            const size_t selected_adapter_idx = scorer(gpus);
            if (selected_adapter_idx != -1)
            {
                m_selected_adapter = m_adapters[selected_adapter_idx];

                g_gpu_info.adapter_vendor_id = conversions::to_gpu_vendor_id(m_selected_adapter->description().desc.VendorId);
                g_gpu_info.adapter_device_id = m_selected_adapter->description().desc.DeviceId;
                g_gpu_info.adapter_revision = m_selected_adapter->description().desc.Revision;
                g_gpu_info.adapter_name = m_selected_adapter->description().desc.Description;
                g_gpu_info.adapter_is_integrated = m_selected_adapter->description().is_integrated;
            }
            else
            {
                log::error("Failed to choose D3D12 Adapter.");
            }
        }

        //-------------------------------------------------------------------------
        bool adapter_manager::load_adapters(s32 gpu_preference)
        {
            // As a fallback we use the IDXGIFactory4 to query all adapters
            wrl::com_ptr<IDXGIFactory4> factory4;
            helpers::safe_create_dxgi_factory(factory4.GetAddressOf());
            if (!factory4)
            {
                log::error("Unable to create IDXGIFactory4, check system support");
                return false;
            }

            // When IDXGIFactory6 is present we can finetune our search when we query adapters
            // It is not a blocking failure when the system does not support this API
            wrl::com_ptr<IDXGIFactory6> factory6;
            factory4->QueryInterface(__uuidof(IDXGIFactory6), (void**)factory6.GetAddressOf());
            if (!factory6)
            {
                log::warn("Unable to query IDXGIFactory6, using IDXGIFactory4");
                m_adapters = get_adapters(get_enumaration_function_factory2(factory4));
            }
            else
            {
                m_adapters = get_adapters(get_enumaration_function_factory6(factory6, gpu_preference == -1 ? DXGI_GPU_PREFERENCE_UNSPECIFIED : (DXGI_GPU_PREFERENCE)gpu_preference));
            }

            return m_adapters.empty() == false; // NOLINT(readability-simplify-boolean-expr)
        }

        //-------------------------------------------------------------------------
        adapter_ptr adapter_manager::selected() const
        {
            CERA_ASSERT_X(m_selected_adapter, "No adapter selected. Call \" select(u32 adapterID) \" first");

            return m_selected_adapter;
        }
        //-------------------------------------------------------------------------
        adapter_ptr adapter_manager::first() const
        {
            CERA_ASSERT_X(!m_adapters.empty(), "No adapters found");

            return m_adapters.front();
        }
        //-------------------------------------------------------------------------
        const adapter_vec& adapter_manager::all() const
        {
            CERA_ASSERT_X(!m_adapters.empty(), "No adapters found");

            return m_adapters;
        }
    } // namespace renderer
} // namespace cera