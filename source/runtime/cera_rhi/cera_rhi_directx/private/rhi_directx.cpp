#include "rhi_factory.h"
#include "rhi_globals.h"
#include "rhi_directx.h"

#include "rhi_windows_config.h"
#include "rhi_windows_target_settings.h"

#include "common/rhi_feature_level.h"
#include "common/rhi_shader_platform.h"

#include "util/assert.h"

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
                    feature_levels.push_back(FDataDrivenShaderPlatformInfo::GetMaxFeatureLevel(shader_platform));
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

            bool is_supported()
            {
                return true;
            }

            std::unique_ptr<rhi> create()
            {
                return std::make_unique<rhi_directx>();
            }
        } // namespace rhi_factory

        void rhi_directx::initialize()
        {
#ifdef CERA_PLATFORM_WINDOWS
            g_shader_platform[feature_level::UNSPECIFIED] = shader_platform::NONE;
#endif
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