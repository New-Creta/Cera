#include "rhi_factory.h"
#include "rhi_globals.h"
#include "rhi_directx.h"
#include "rhi_windows_config.h"
#include "rhi_windows_target_settings.h"

#include "common/rhi_feature_level.h"
#include "common/rhi_shader_platform.h"
#include "common/rhi_data_driven_shader_platform_info.h"
#include "common/rhi_shader_format_names.h"

#include "dxgi/dxgi_adapter_manager.h"
#include "dxgi/dxgi_util.h"

#include "util/assert.h"
#include "util/defines.h"

#include "rhi_directx_device.h"
#include "rhi_directx_call.h"
#include "rhi_directx_viewport.h"

#include "core_platform.h"

#include <vector>
#include <optional>
#include <string>

namespace cera
{
    namespace renderer
    {
        namespace rhi_factory
        {
            namespace internal
            {
                //-------------------------------------------------------------------------
                // Globals
                std::unique_ptr<adapter_manager> g_adapter_manager = nullptr;

                //-------------------------------------------------------------------------
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

                //-------------------------------------------------------------------------
                std::optional<feature_level> handle_unsupported_feature_level(const windows_rhi_type& in_winndows_rhi, const feature_level& in_feature_level, const windows_rhi_config& in_config)
                {
                    if (std::optional<feature_level> fallback_feature_level = in_config.get_next_highest_supported_feature_level(in_winndows_rhi, in_feature_level))
                    {
                        return fallback_feature_level;
                    }

                    return {};
                }
                //-------------------------------------------------------------------------
                std::optional<std::tuple<windows_rhi_type, feature_level>> handle_unsupported_rhi(const windows_rhi_type& in_winndows_rhi, const feature_level& in_feature_level, const windows_rhi_config& in_config)
                {
                    if (in_winndows_rhi == windows_rhi_type::D3D12)
                    {
                        if (std::optional<feature_level> d3d11_feature_level = in_config.get_highest_supported_feature_level(windows_rhi_type::D3D11))
                        {
                            return {std::make_tuple(windows_rhi_type::D3D11, *d3d11_feature_level)};
                        }
                    }

                    log::info("RHI {0} is not supported on your system.", conversions::to_string(in_winndows_rhi));

                    if (in_winndows_rhi == windows_rhi_type::D3D12)
                    {
                        log::error("DirectX 12 is not supported on your system.");
                        platform::request_engine_exit("HandleUnsupportedRHI.D3D12");
                    }

                    if (in_winndows_rhi == windows_rhi_type::D3D11)
                    {
                        log::error("A D3D11-compatible GPU (Feature Level 11.0, Shader Model 5.0) is required to run the engine.");
                        platform::request_engine_exit("HandleUnsupportedRHI.D3D11");
                    }

                    if (in_winndows_rhi == windows_rhi_type::OPENGL)
                    {
                        log::error("OpenGL 4.3 is required to run the engine.");
                        platform::request_engine_exit("HandleUnsupportedRHI.OpenGL");
                    }

                    return {};
                }

                //-------------------------------------------------------------------------
                std::optional<windows_rhi_type> parse_windows_default_rhi()
                {
                    class find_rhi
                    {
                      public:
                        find_rhi(const std::string_view input) : m_input(input)
                        {
                        }

                        bool operator()(const std::string_view rhi)
                        {
                            return m_input == rhi;
                        }

                      private:
                        std::string_view m_input;
                    };

                    const auto& settings = get_windows_target_settings();

                    auto it = settings.find("DefaultGraphicsRHI");
                    if (it != settings.end())
                    {
                        if (std::find_if(it->second.begin(), it->second.end(), find_rhi(g_default_graphics_rhi_dx11)) != it->second.end())
                        {
                            return windows_rhi_type::D3D11;
                        }
                        else if (std::find_if(it->second.begin(), it->second.end(), find_rhi(g_default_graphics_rhi_dx12)) != it->second.end())
                        {
                            return windows_rhi_type::D3D12;
                        }
                        else if (std::find_if(it->second.begin(), it->second.end(), find_rhi(g_default_graphics_rhi_opengl)) != it->second.end())
                        {
                            return windows_rhi_type::OPENGL;
                        }
                        else
                        {
                            log::error("Unrecognized setting '{}' for DefaultGraphicsRHI", it->second);
                        }
                    }

                    return {};
                }
                //-------------------------------------------------------------------------
                std::vector<shader_platform> parse_shader_platforms_config(const std::string& settings_name)
                {
                    const auto& settings = get_windows_target_settings();

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
                //-------------------------------------------------------------------------
                windows_rhi parse_windows_rhi(const std::string& settings_name)
                {
                    windows_rhi config;

                    config.shader_platforms = parse_shader_platforms_config(settings_name);
                    config.feature_levels = feature_levels_from_shader_platforms(config.shader_platforms);

                    return config;
                }
                //-------------------------------------------------------------------------
                windows_rhi_config parse_windows_rhi_config()
                {
                    windows_rhi_config config;

                    config.default_rhi = parse_windows_default_rhi();

                    config.rhi_configs[(s32)windows_rhi_type::D3D11] = parse_windows_rhi("D3D11TargetedShaderFormats");
                    config.rhi_configs[(s32)windows_rhi_type::D3D12] = parse_windows_rhi("D3D12TargetedShaderFormats");
                    config.rhi_configs[(s32)windows_rhi_type::OPENGL] = parse_windows_rhi("OpenGLTargetedShaderFormats");

                    return config;
                }

                //-------------------------------------------------------------------------
                std::optional<windows_rhi_type> choose_preferred_rhi(windows_rhi_type in_default_rhi)
                {
                    std::optional<windows_rhi_type> rhi_preference{};

                    // If we are in game, there is a separate setting that can make it prefer D3D12 over D3D11 (but not over other RHIs).
                    if ((in_default_rhi == windows_rhi_type::D3D11 || in_default_rhi == windows_rhi_type::D3D12))
                    {
                        std::string preferred_rhi_name = g_preferred_rhi_name;
                        if (!preferred_rhi_name.empty())
                        {
                            log::info("Found D3DRHIPreference PreferredRHI: {}", preferred_rhi_name);

                            if (preferred_rhi_name == std::string("dx12"))
                            {
                                rhi_preference = windows_rhi_type::D3D12;
                            }
                            else if (preferred_rhi_name == std::string("dx11"))
                            {
                                rhi_preference = windows_rhi_type::D3D11;
                            }
                            else if (preferred_rhi_name == std::string("opengl"))
                            {
                                rhi_preference = windows_rhi_type::OPENGL;
                            }
                            else
                            {
                                log::error("unknown RHI name \"{}\" in game user settings, using default", preferred_rhi_name);
                            }
                        }
                    }

                    return rhi_preference;
                }
                //-------------------------------------------------------------------------
                std::optional<feature_level> choose_preferred_feature_level()
                {
                    std::optional<feature_level> preferred_feature_level;

                    std::string preferred_feature_level_name = g_preferred_rhi_shader_format_name;
                    if (!preferred_feature_level_name.empty())
                    {
                        log::info("Found D3DRHIPreference preferred_feature_level: {}", preferred_feature_level_name);

                        if (preferred_feature_level_name == shader_formats::g_name_d3d_sm5)
                        {
                            preferred_feature_level = feature_level::D3D_SM5;
                        }
                        else if (preferred_feature_level_name == shader_formats::g_name_d3d_sm6)
                        {
                            preferred_feature_level = feature_level::D3D_SM6;
                        }
                        else if (preferred_feature_level_name == shader_formats::g_name_ogl_sm5)
                        {
                            preferred_feature_level = feature_level::OGL_SM5;
                        }
                        else
                        {
                            log::error("Unknown feature level name \"{}\" in game user settings, using default", preferred_feature_level_name);
                        }
                    }

                    return preferred_feature_level;
                }
                //-------------------------------------------------------------------------
                windows_rhi_type choose_default_rhi(const windows_rhi_config& config)
                {
                    // Default graphics RHI is the main project setting that governs the choice, so it takes the priority
                    std::optional<windows_rhi_type> config_default = config.default_rhi;
                    if (config_default)
                    {
                        return *config_default;
                    }

                    return windows_rhi_type::OPENGL;
                }
                //-------------------------------------------------------------------------
                feature_level choose_default_feature_level_for_rhi(windows_rhi_type in_windows_rhi)
                {
                    switch (in_windows_rhi)
                    {
                    case windows_rhi_type::D3D11:
                        return feature_level::D3D_SM5;
                    case windows_rhi_type::D3D12:
                        return feature_level::D3D_SM6;
                    case windows_rhi_type::OPENGL:
                        return feature_level::OGL_SM5;
                    default:
                        CERA_ASSERT("Unsupported RHI type was passed");
                        return feature_level::D3D_SM5;
                    }
                }
                //-------------------------------------------------------------------------
                feature_level choose_feature_level(windows_rhi_type chosen_rhi, const windows_rhi_config& config)
                {
                    std::optional<feature_level> feature_level = choose_preferred_feature_level();

                    if (!feature_level || !config.is_feature_level_targted(chosen_rhi, *feature_level))
                    {
                        feature_level = config.get_highest_supported_feature_level(chosen_rhi);

                        // If we were forced to a specific RHI while not forced to a specific feature level and the project isn't configured for it, find the default Feature Level for that RHI
                        if (!feature_level)
                        {
                            feature_level = choose_default_feature_level_for_rhi(chosen_rhi);

                            {
                                log::info("User requested RHI '{0}' but that is not supported by this project's data. Defaulting to Feature Level '{1}'.", conversions::to_string(chosen_rhi), conversions::to_string(*feature_level));
                            }
                        }
                        else
                        {
                            log::info("Using Highest Feature Level of {0}: {1}", conversions::to_string(chosen_rhi), conversions::to_string(*feature_level));
                        }
                    }

                    return *feature_level;
                }

                //-------------------------------------------------------------------------
                bool does_any_adapter_support_sm6(const adapter_vec& adapters)
                {
                    for (const auto& adapter : adapters)
                    {
                        auto desc = adapter->description();
                        if (desc.is_valid() && desc.max_rhi_feature_level >= feature_level::D3D_SM6)
                        {
                            return true;
                        }
                    }

                    return false;
                }

                //-------------------------------------------------------------------------
                std::unique_ptr<rhi> create_rhi(feature_level in_feature_level)
                {
                    g_shader_platform[(s32)feature_level::OGL_SM5] = shader_platform::OGL_SM5;
                    g_shader_platform[(s32)feature_level::D3D_SM5] = shader_platform::D3D_SM5;
                    if (does_any_adapter_support_sm6(g_adapter_manager->all()))
                    {
                        g_shader_platform[(s32)feature_level::D3D_SM6] = shader_platform::D3D_SM6;
                    }

                    g_max_feature_level = in_feature_level;
                    CERA_ASSERT_X(g_max_feature_level < feature_level::NUM, "Given feature level to create RHI was invalid");
                    g_max_shader_platform = g_shader_platform[(s32)in_feature_level];
                    CERA_ASSERT_X(g_max_feature_level < shader_platform::NUM, "Given shader platform to create RHI was invalid");

                    log::info("Creating D3D12 RHI with Max Feature Level {0}", conversions::to_string(g_max_feature_level));

                    return std::make_unique<rhi_directx>(g_adapter_manager->selected());
                }
            } // namespace internal

            //-------------------------------------------------------------------------
            bool is_supported(feature_level in_feature_level)
            {
                // Windows version 15063 is Windows 1703 aka "Windows Creator Update"
                // This is the first version that supports ID3D12Device2 which is our minimum runtime device version.
                if (!platform::verify_windows_version(10, 0, 15063))
                {
                    log::error("Missing full support for Direct3D 12. Update to Windows 1703 or newer for D3D12 support.");
                    return false;
                }

                // Define a lambda function to score and select the GPU with the highest dedicated video memory (VRAM).
                auto scorer = [](const std::vector<adapter_description>& gpus) 
                {
                    // Use std::max_element to find the GPU with the maximum VRAM.
                    auto it = std::max_element(gpus.cbegin(), gpus.cend(), [](const adapter_description& lhs, const adapter_description& rhs) 
                    {
                        const size_t lhs_vram = lhs.desc.DedicatedVideoMemory;
                        const size_t rhs_vram = rhs.desc.DedicatedVideoMemory;

                        return rhs_vram > lhs_vram;
                    });

                    // Return the index of the GPU with the highest VRAM, or -1 if no valid GPU is found.
                    return it != gpus.cend() ? std::distance(gpus.cbegin(), it) : -1;
                };

                // Initialize the adapter manager with the scoring function and high-performance GPU preference.
                internal::g_adapter_manager = std::make_unique<adapter_manager>(scorer, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE);

                // Retrieve the selected adapter from the adapter manager.
                std::shared_ptr<adapter> selected_adapter = internal::g_adapter_manager->selected();
                if (!selected_adapter || !selected_adapter->description().is_valid())
                {
                    log::error("Selected DXGIAdapter is invalid");
                    return false;
                }

                // Check if the selected IDXGIAdapter supports the required feature level.
                const adapter_description& desc = selected_adapter->description();                
                if (desc.is_valid() && desc.max_rhi_feature_level >= in_feature_level)
                {
                    return true;
                }

                std::string supported_feature_level = conversions::to_string(desc.max_rhi_feature_level);
                std::string requested_feature_level = conversions::to_string(in_feature_level);

                log::error("Adapter only supports up to Feature Level '{0}', requested Feature Level was '{1}'", supported_feature_level, requested_feature_level);

                return false;
            }
            
            //-------------------------------------------------------------------------
            std::unique_ptr<rhi> create()
            {
                // Make sure the data driven shader platform is initialized before we try and use it
                data_driven_shader_platform_info::initialize();

                // Parse the configuration specific to Windows RHI.
                windows_rhi_config config = internal::parse_windows_rhi_config();

                // Choose the default RHI type based on the configuration.
                auto default_rhi_type = internal::choose_default_rhi(config);
                // Choose the preferred RHI type if available.
                auto preferred_rhi = internal::choose_preferred_rhi(default_rhi_type);

                // Set the chosen RHI type, default to the default RHI type.
                windows_rhi_type chosen_rhi = default_rhi_type;
                if (preferred_rhi)
                {
                    chosen_rhi = *preferred_rhi;

                    log::info("Using Preferred RHI: %s"), conversions::to_string(chosen_rhi);
                }
                else
                {
                    log::info("Using Default RHI: %s"), conversions::to_string(chosen_rhi);
                }

                // Determine the desired feature level based on the chosen RHI and configuration.
                feature_level desired_feature_level = internal::choose_feature_level(chosen_rhi, config);

                // Attempt to load the dynamic RHI module.
                std::optional<feature_level> fallback_feature_level = {};
                do
                {
                    // Check if the desired feature level is supported by the RHI.
                    if (rhi_factory::is_supported(desired_feature_level))
                    {
                        log::info("RHI {0} with Feature Level {1} is supported and will be used.", conversions::to_string(chosen_rhi), conversions::to_string(desired_feature_level));
                        return internal::create_rhi(desired_feature_level);
                    }

                    // Save the current RHI and feature level before attempting fallback.
                    const windows_rhi_type previous_rhi = chosen_rhi;
                    const feature_level previous_feature_level = desired_feature_level;

                    // Handle the scenario where the desired feature level is not supported.
                    fallback_feature_level = internal::handle_unsupported_feature_level(chosen_rhi, desired_feature_level, config);

                    // If no fallback feature level is available, handle unsupported RHI.
                    if (!fallback_feature_level)
                    {
                        auto next = internal::handle_unsupported_rhi(chosen_rhi, desired_feature_level, config);
                        if (next)
                        {
                            auto[chosen_rhi, fallback_feature_level] = *next;
                        }
                        else
                        {
                            log::error("RHI {0} with Feature Level {1} is not supported on your system. No RHI was supported, failing initialization.", conversions::to_string(chosen_rhi), conversions::to_string(desired_feature_level));
                            return nullptr;
                        }
                    }
                    else
                    {
                        log::info("RHI {0} with Feature Level {1} is not supported on your system, attempting to fall back to RHI {2} with Feature Level {3}"
                        ,conversions::to_string(previous_rhi)
                        ,conversions::to_string(previous_feature_level)
                        ,conversions::to_string(chosen_rhi)
                        ,conversions::to_string(desired_feature_level));
                    }

                } while (!fallback_feature_level);

                // This part of the code should not be reached; log an error if it is.
                log::error("Unreachable code ...");
                return nullptr;
            }

            //-------------------------------------------------------------------------
            std::shared_ptr<rhi_viewport> create_viewport(void* in_window_handle, s32 size_x, s32 size_y, bool is_fullscreen)
            {
                return std::make_shared<d3d12_rhi_viewport>(in_window_handle, size_x, size_y, in_fullscreen);
            }

            //-------------------------------------------------------------------------
            void resize_viewport(std::shared_ptr<rhi_viewport> in_viewport, s32 size_x, s32 size_y, bool is_fullscreen)
            {

            }

        } // namespace rhi_factory

        //-------------------------------------------------------------------------
        rhi_directx::rhi_directx(const std::shared_ptr<adapter>& adapter) 
            : m_adapter(adapter)
        {

        }

        //-------------------------------------------------------------------------
        void rhi_directx::initialize()
        {
            CERA_ASSERT_X(!g_rhi_initialized, "rhi was already initialized!");

            CERA_ASSERT_X(m_adapter->description().is_valid());

            m_root_device = d3d12_device::create(m_adapter, g_is_debug_layer_enabled);

            g_rhi_initialized = true;
        }
        
        //-------------------------------------------------------------------------
        void rhi_directx::shutdown()
        {
            m_root_device.reset();

            g_rhi_initialized = false;
        }

        //-------------------------------------------------------------------------
        adapter* rhi_directx::get_adapter()
        {
            return m_adapter.get();
        }
        //-------------------------------------------------------------------------
        const adapter* rhi_directx::get_adapter() const
        {
            return m_adapter.get();
        }
        //-------------------------------------------------------------------------
        d3d12_device* rhi_directx::get_device()
        {
            return m_root_device.get();
        }
        //-------------------------------------------------------------------------
        const d3d12_device* rhi_directx::get_device() const
        {
            return m_root_device.get();
        }

    } // namespace renderer
} // namespace cera