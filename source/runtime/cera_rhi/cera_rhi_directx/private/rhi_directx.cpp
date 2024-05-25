#include "rhi_factory.h"
#include "rhi_globals.h"
#include "rhi_directx.h"
#include "rhi_windows_config.h"
#include "rhi_windows_target_settings.h"

#include "common/rhi_feature_level.h"
#include "common/rhi_shader_platform.h"
#include "common/rhi_data_driven_shader_platform_info.h"
#include "common/rhi_shader_format_names.h"

#include "util/assert.h"
#include "util/defines.h"

#include "gpu_helper.h"
#include "gpu_description.h"

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
            std::optional<feature_level> handle_unsupported_feature_level(const windows_rhi_type& in_winndows_rhi, const feature_level& in_feature_level, const windows_rhi_config& in_config)
            {
                if (std::optional<feature_level> fallback_feature_level = in_config.get_next_highest_supported_feature_level(in_winndows_rhi, in_feature_level))
                {
                    return fallback_feature_level;
                }

                return {};
            }

            std::optional<std::tuple<windows_rhi_type, feature_level>> handle_unsupported_rhi(const windows_rhi_type& in_winndows_rhi, const feature_level& in_feature_level, const windows_rhi_config& in_config)
            {
                if (in_winndows_rhi == windows_rhi_type::D3D12)
                {
                    if (std::optional<feature_level> d3d11_feature_level = in_config.get_highest_supported_feature_level(windows_rhi_type::D3D11))
                    {
                        return { std::make_tuple(windows_rhi_type::D3D11, *d3d11_feature_level) };
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

            bool is_supported(feature_level in_feature_level)
            {
                return true;
            }

            std::unique_ptr<rhi> create()
            {
                // Make sure the data driven shader platform is initialized before we try and use it
                data_driven_shader_platform_info::initialize();

                windows_rhi_config config = parse_windows_rhi_config();

                auto default_rhi_type = choose_default_rhi(config);
                auto preferred_rhi = choose_preferred_rhi(default_rhi_type);

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

                feature_level desired_feature_level = choose_feature_level(chosen_rhi, config);

                // Load the dynamic RHI module.

                std::optional<feature_level> next_feature_level = {};
                do
                {
                    if (rhi_factory::is_supported(desired_feature_level))
                    {
                        log::info("RHI {0} with Feature Level {1} is supported and will be used.", conversions::to_string(chosen_rhi), conversions::to_string(desired_feature_level));

                        return rhi_factory::create();
                    }

                    const windows_rhi_type previous_rhi = chosen_rhi;
                    const feature_level previous_feature_level = desired_feature_level;

                    next_feature_level = handle_unsupported_feature_level(chosen_rhi, desired_feature_level, config);

                    if (!next_feature_level)
                    {
                        next_feature_level = handle_unsupported_rhi(chosen_rhi, desired_feature_level, config);
                    }

                    if (next_feature_level)
                    {
                        log::info("RHI {0} with Feature Level {1} is not supported on your system, attempting to fall back to RHI {2} with Feature Level {3}"
                        ,conversions::to_string(previous_rhi)
                        ,conversions::to_string(previous_feature_level)
                        ,conversions::to_string(chosen_rhi)
                        ,conversions::to_string(desired_feature_level));
                    }
                } while (next_feature_level);

                log::info("RHI {0} with Feature Level {1} is not supported on your system. No RHI was supported, failing initialization.", conversions::to_string(chosen_rhi), conversions::to_string(desired_feature_level));

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