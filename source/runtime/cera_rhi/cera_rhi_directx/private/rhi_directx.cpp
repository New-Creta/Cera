#include "rhi_factory.h"
#include "rhi_globals.h"
#include "rhi_directx.h"

#include "rhi_windows_config.h"
#include "rhi_windows_target_settings.h"

#include "common/rhi_feature_level.h"
#include "common/rhi_shader_platform.h"
#include "common/rhi_data_driven_shader_platform_info.h"

#include "util/assert.h"

#include "gpu_helper.h"
#include "gpu_description.h"

#include <vector>
#include <optional>
#include <string>

namespace cera
{
    namespace renderer
    {
        namespace rhi_factory
        {
            std::optional<windows_rhi_type> parse_windows_default_rhi()
            {
                
            }

            std::vector<shader_platform> parse_shader_platforms_config(const std::string& settings_name)
            {
                auto& settings = get_windows_target_settings();

                auto it = settings.find(settings_name);
                if (it != settings.end())
                {
                    const std::vector<std::string>& target_shader_formats = it->second;

                    std::vector<shader_platform> shader_platforms;
                    shader_platforms.reserve(target_shader_formats.size());

                    for (const std::string& shader_format : target_shader_formats)
                    {
                        shader_platforms.push_back(shader_format_to_shader_platform(shader_format));
                    }

                    return shader_platforms;
                }

                CERA_ASSERT("Unknown Windows Target Setting");
                return {};
            }

            std::vector<feature_level> feature_levels_from_shader_platforms(const std::vector<shader_platform>& shader_platforms)
            {
                std::vector<feature_level> feature_levels;
                feature_levels.reserve(shader_platforms.size());

                for (shader_platform shader_platform : shader_platforms)
                {
                    feature_levels.push_back(data_driven_shader_platform_info::get_max_feature_level(shader_platform));
                }

                return feature_levels;
            }

            windows_rhi parse_windows_rhi(const std::string& settings_name)
            {
                windows_rhi config;

                config.shader_platforms = parse_shader_platforms_config(settings_name);
                config.feature_levels = feature_levels_from_shader_platforms(config.shader_platforms);

                return config;
            }

            windows_rhi_config parse_windows_rhi_config()
            {
                windows_rhi_config config;

                config.default_rhi = parse_windows_default_rhi();

                config.rhi_configs[(s32)windows_rhi_type::D3D11] = parse_windows_rhi("D3D11TargetedShaderFormats");
                config.rhi_configs[(s32)windows_rhi_type::D3D12] = parse_windows_rhi("D3D12TargetedShaderFormats");
                config.rhi_configs[(s32)windows_rhi_type::OPENGL] = parse_windows_rhi("OpenGLTargetedShaderFormats");
                                
                return config;
            }

            static bool ShouldDevicePreferSM5(uint32 DeviceId)
            {
                uint32 SM5PreferedDeviceIds[] = {
                    0x1B80, // "NVIDIA GeForce GTX 1080"
                    0x1B81, // "NVIDIA GeForce GTX 1070"
                    0x1B82, // "NVIDIA GeForce GTX 1070 Ti"
                    0x1B83, // "NVIDIA GeForce GTX 1060 6GB"
                    0x1B84, // "NVIDIA GeForce GTX 1060 3GB"
                    0x1C01, // "NVIDIA GeForce GTX 1050 Ti"
                    0x1C02, // "NVIDIA GeForce GTX 1060 3GB"
                    0x1C03, // "NVIDIA GeForce GTX 1060 6GB"
                    0x1C04, // "NVIDIA GeForce GTX 1060 5GB"
                    0x1C06, // "NVIDIA GeForce GTX 1060 6GB"
                    0x1C08, // "NVIDIA GeForce GTX 1050"
                    0x1C81, // "NVIDIA GeForce GTX 1050"
                    0x1C82, // "NVIDIA GeForce GTX 1050 Ti"
                    0x1C83, // "NVIDIA GeForce GTX 1050"
                    0x1B06, // "NVIDIA GeForce GTX 1080 Ti"
                };

                for (int Index = 0; Index < UE_ARRAY_COUNT(SM5PreferedDeviceIds); ++Index)
                {
                    if (DeviceId == SM5PreferedDeviceIds[Index])
                    {
                        return true;
                    }
                }

                return false;
            }

            // Whether a SM6 capable device should always default to SM6
            // regardless of other heuristics suggesting otherwise.
            static bool ShouldDevicePreferSM6(u32 SM6CapableDeviceId)
            {
                TArray<FString> SM6PreferredDeviceIds;
                GConfig->GetArray(TEXT("D3D12_SM6"), TEXT("SM6PreferredGPUDeviceIDs"), SM6PreferredDeviceIds, GEngineIni);

                for (const FString& PreferredDeviceID : SM6PreferredDeviceIds)
                {
                    if (SM6CapableDeviceId == FCString::Strtoi(*PreferredDeviceID, nullptr, 10) || SM6CapableDeviceId == FCString::Strtoi(*PreferredDeviceID, nullptr, 16))
                    {
                        return true;
                    }
                }

                return false;
            }

            static bool is_rhi_allowed_as_default(windows_rhi_type in_rhi, feature_level in_feature_level)
            {
                static const GpuDescription best_gpu_desc = find_best_gpu();

                if (in_rhi == windows_rhi_type::D3D12 && in_feature_level == feature_level::D3D_SM6)
                {
                    if (ShouldDevicePreferSM6(BestGPUInfo.DeviceId))
                    {
                        return true;
                    }
                }

                bool bAllowed = true;
                if (InRHI == EWindowsRHI::D3D12 && InFeatureLevel >= ERHIFeatureLevel::SM6)
                {
                    int32 MinDedicatedMemoryMB = 0;
                    if (GConfig->GetInt(TEXT("D3D12_SM6"), TEXT("MinDedicatedMemory"), MinDedicatedMemoryMB, GEngineIni))
                    {
                        const uint64 MinDedicatedMemory = static_cast<uint64>(MinDedicatedMemoryMB) << 20;
                        bAllowed &= BestGPUInfo.DedicatedVideoMemory >= MinDedicatedMemory;
                    }

                    bool bUseSM5PreferredGPUList = true;
                    GConfig->GetBool(TEXT("D3D12_SM6"), TEXT("bUseSM5PreferredGPUList"), bUseSM5PreferredGPUList, GEngineIni);
                    if (bUseSM5PreferredGPUList)
                    {
                        bAllowed &= !ShouldDevicePreferSM5(BestGPUInfo.DeviceId);
                    }
                }
                return bAllowed;
            }

            static std::optional<feature_level> choose_default_feature_level(windows_rhi_type in_rhi, const windows_rhi_config& config)
            {
                std::optional<feature_level> highest_feature_level = config.get_highest_supported_feature_level(in_rhi);
                while (highest_feature_level)
                {
                    if (IsRHIAllowedAsDefault(InRHI, HighestFL.GetValue()))
                    {
                        return HighestFL;
                    }
                    HighestFL = Config.GetNextHighestTargetedFeatureLevel(InRHI, HighestFL.GetValue());
                }

                return TOptional<ERHIFeatureLevel::Type>();
            }

            // Choose the default from DefaultGraphicsRHI or TargetedRHIs. DefaultGraphicsRHI has precedence.
            static windows_rhi_type choose_default_rhi(const windows_rhi_config& config)
            {
                // Default graphics RHI is the main project setting that governs the choice, so it takes the priority
                std::optional<windows_rhi_type> config_default = config.default_rhi
                if (config_default)
                {
                    return *config_default;
                }

                // Find the first RHI with configured support based on the order above
                for (windows_rhi_type default_rhi : g_search_order)
                {
                    if (std::optional<feature_level> highest_feature_level = choose_default_feature_level(default_rhi, config))
                    {
                        return default_rhi;
                    }
                }

                return windows_rhi_type::D3D11;
            }

            bool is_supported()
            {
                return true;
            }

            std::unique_ptr<rhi> create()
            {
                // Make sure the data driven shader platform is initialized before we try and use it
                data_driven_shader_platform_info::initialize();

                windows_rhi_config config = parse_windows_rhi_config();

                // RHI is chosen by the project settings (first DefaultGraphicsRHI, then TargetedRHIs are consulted, "Default" maps to D3D12).
                // After this, a separate game-only setting (does not affect editor) bPreferD3D12InGame selects between D3D12 or D3D11 (but will not have any effect if Vulkan or OpenGL are chosen).
                // Commandline switches apply after this and can force an arbitrary RHIs. If RHI isn't supported, the game will refuse to start.

                EWindowsRHI DefaultRHI = ChooseDefaultRHI(Config);
                const TOptional<EWindowsRHI> PreferredRHI = ChoosePreferredRHI(DefaultRHI);
                const TOptional<ERHIFeatureLevel::Type> ForcedFeatureLevel = GetForcedFeatureLevel();
                const TOptional<EWindowsRHI> ForcedRHI = ChooseForcedRHI(ForcedFeatureLevel, Config);

                EWindowsRHI ChosenRHI = DefaultRHI;
                if (ForcedRHI)
                {
                    ChosenRHI = ForcedRHI.GetValue();

                    UE_LOG(LogRHI, Log, TEXT("Using Forced RHI: %s"), GetLogName(ChosenRHI));
                }
                else if (PreferredRHI)
                {
                    ChosenRHI = PreferredRHI.GetValue();

                    UE_LOG(LogRHI, Log, TEXT("Using Preferred RHI: %s"), GetLogName(ChosenRHI));
                }
                else
                {
                    UE_LOG(LogRHI, Log, TEXT("Using Default RHI: %s"), GetLogName(ChosenRHI));
                }

                DesiredFeatureLevel = ChooseFeatureLevel(ChosenRHI, ForcedRHI, ForcedFeatureLevel, Config);

                // Load the dynamic RHI module.

                bool bTryWithNewConfig = false;
                do
                {
                    const FString RHIName = GetRHINameFromWindowsRHI(ChosenRHI, DesiredFeatureLevel);
                    FApp::SetGraphicsRHI(RHIName);

                    const TCHAR* ModuleName = ModuleNameFromWindowsRHI(ChosenRHI);

                    UE_LOG(LogRHI, Log, TEXT("Loading RHI module %s"), ModuleName);

                    IDynamicRHIModule* DynamicRHIModule = FModuleManager::LoadModulePtr<IDynamicRHIModule>(ModuleName);

                    UE_LOG(LogRHI, Log, TEXT("Checking if RHI %s with Feature Level %s is supported by your system."), GetLogName(ChosenRHI), GetLogName(DesiredFeatureLevel));

                    if (DynamicRHIModule && DynamicRHIModule->IsSupported(DesiredFeatureLevel))
                    {
                        UE_LOG(LogRHI, Log, TEXT("RHI %s with Feature Level %s is supported and will be used."), GetLogName(ChosenRHI), GetLogName(DesiredFeatureLevel));

                        LoadedRHIModuleName = ModuleName;
                        return DynamicRHIModule;
                    }

                    const EWindowsRHI PreviousRHI = ChosenRHI;
                    const ERHIFeatureLevel::Type PreviousFeatureLevel = DesiredFeatureLevel;

                    bTryWithNewConfig = HandleUnsupportedFeatureLevel(ChosenRHI, DesiredFeatureLevel, ForcedFeatureLevel, Config);

                    if (!bTryWithNewConfig)
                    {
                        bTryWithNewConfig = HandleUnsupportedRHI(ChosenRHI, DesiredFeatureLevel, ForcedRHI, Config);
                    }

                    if (bTryWithNewConfig)
                    {
                        UE_LOG(LogRHI, Log, TEXT("RHI %s with Feature Level %s is not supported on your system, attempting to fall back to RHI %s with Feature Level %s"), GetLogName(PreviousRHI), GetLogName(PreviousFeatureLevel),
                               GetLogName(ChosenRHI), GetLogName(DesiredFeatureLevel));
                    }
                } while (bTryWithNewConfig);

                UE_LOG(LogRHI, Log, TEXT("RHI %s with Feature Level %s is not supported on your system. No RHI was supported, failing initialization."), GetLogName(ChosenRHI), GetLogName(DesiredFeatureLevel));

                return nullptr;

                return std::make_unique<rhi_directx>();
            }
        } // namespace rhi_factory

        void rhi_directx::initialize()
        {
            CERA_ASSERT_X(!g_rhi_initialized, "rhi was already initialized!");

            g_rhi_initialized = true;
        }

        void rhi_directx::post_initialize()
        {
            // Nothing to implement
        }

        void rhi_directx::shutdown()
        {
            // Nothing to implement
        }

        std::shared_ptr<rhi_byte_address_buffer> rhi_directx::create_byte_address_buffer(size_t /*buffer_size*/)
        {
            return nullptr;
        }
        std::shared_ptr<rhi_texture> rhi_directx::create_texture(const rhi_texture_desc& /*desc*/, const rhi_clear_value_desc& /*clear_value*/)
        {
            return nullptr;
        }
        std::shared_ptr<rhi_index_buffer> rhi_directx::create_index_buffer(size_t /*num_indices*/, rhi_format /*index_format*/)
        {
            return nullptr;
        }
        std::shared_ptr<rhi_vertex_buffer> rhi_directx::create_vertex_buffer(size_t /*num_vertices*/, size_t /*vertex_stride*/)
        {
            return nullptr;
        }
        std::shared_ptr<rhi_pipeline_state_object> rhi_directx::create_pipeline_state_object(const rhi_pipeline_state_object_desc& /*pipeline_state_stream*/)
        {
            return nullptr;
        }
        std::shared_ptr<rhi_shader_resource_view> rhi_directx::create_shader_resource_view(const std::shared_ptr<rhi_resource>& /*in_resource*/, const rhi_shader_resource_view_desc& /*srv*/)
        {
            return nullptr;
        }
        std::shared_ptr<rhi_unordered_address_view> rhi_directx::create_unordered_access_view(const std::shared_ptr<rhi_resource>& /*in_resource*/, const std::shared_ptr<rhi_resource>& /*in_counter_resource*/,
                                                                                           const rhi_unordered_access_view_desc& /*uav*/)
        {
            return nullptr;
        }
    } // namespace renderer
} // namespace cera