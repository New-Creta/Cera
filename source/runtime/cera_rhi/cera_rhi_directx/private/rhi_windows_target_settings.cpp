#include "rhi_windows_target_settings.h"

#include "common/rhi_shader_platform.h"
#include "common/rhi_shader_format_names.h"

#include "util/assert.h"

namespace cera
{
    namespace renderer
    {
        namespace config
        {
            /**
             * This needs to be converted to a settings file in the end
             */
            static const std::string g_d3d11_target_shader_format_setting_key                   = "D3D11TargetedShaderFormats";
            static const std::string g_d3d12_target_shader_format_setting_key                   = "D3D12TargetedShaderFormats";
            static const std::string g_opengl_target_shader_format_setting_key                  = "OpenGLTargetedShaderFormats";

            static const std::vector<std::string> g_d3d11_target_shader_format_setting_value    = {shader_formats::g_name_d3d_sm5};
            static const std::vector<std::string> g_d3d12_target_shader_format_setting_value    = {shader_formats::g_name_d3d_sm5, shader_formats::g_name_d3d_sm6};
            static const std::vector<std::string> g_opengl_target_shader_format_setting_value   = {shader_formats::g_name_ogl_sm5};
        } // namespace config

        const std::unordered_map<std::string, std::vector<std::string>>& get_windows_target_settings()
        {
            static std::unordered_map<std::string, std::vector<std::string>> windows_target_settings = {
                {config::g_d3d11_target_shader_format_setting_key, config::g_d3d11_target_shader_format_setting_value},
                {config::g_d3d12_target_shader_format_setting_key, config::g_d3d12_target_shader_format_setting_value},
                {config::g_opengl_target_shader_format_setting_key, config::g_opengl_target_shader_format_setting_value},
            };

            return windows_target_settings;
        }

        shader_platform shader_format_to_shader_platform(const std::string& shader_format)
        {
            if (shader_format == shader_formats::g_name_d3d_sm5)    return shader_platform::D3D_SM5;
            if (shader_format == shader_formats::g_name_d3d_sm6)    return shader_platform::D3D_SM6;
            if (shader_format == shader_formats::g_name_ogl_sm5)    return shader_platform::OGL_SM5;

            CERA_ASSERT("Invalid shader format given: {}", shader_format);
            return shader_platform::NONE;
        }
    } // namespace renderer
} // namespace cera