#include "rhi_windows_target_settings.h"

#include "common/rhi_shader_platform.h"

#include "util/assert.h"

namespace cera
{
    namespace renderer
    {
        /**
         * This needs to be converted to a settings file in the end
         */
        static const std::string g_d3d11_target_shader_format_setting_key = "D3D11TargetedShaderFormats";
        static const std::string g_d3d12_target_shader_format_setting_key = "D3D12TargetedShaderFormats";
        static const std::string g_opengl_target_shader_format_setting_key = "OpenGLTargetedShaderFormats";

        static const std::string g_name_d3d_sm5 = "D3D_SM5";
        static const std::string g_name_d3d_sm6 = "D3D_SM6";
        static const std::string g_name_gl_sm4_3 = "GL_SM4_3";

        static const std::vector<std::string> g_d3d11_target_shader_format_setting_value = {g_name_d3d_sm5};
        static const std::vector<std::string> g_d3d12_target_shader_format_setting_value = {g_name_d3d_sm5, g_name_d3d_sm6};
        static const std::vector<std::string> g_opengl_target_shader_format_setting_value = {g_name_gl_sm4_3};

        const std::unordered_map<std::string, std::vector<std::string>>& get_windows_target_settings()
        {
            static std::unordered_map<std::string, std::vector<std::string>> windows_target_settings = {
                {g_d3d11_target_shader_format_setting_key, g_d3d11_target_shader_format_setting_value},
                {g_d3d12_target_shader_format_setting_key, g_d3d12_target_shader_format_setting_value},
                {g_opengl_target_shader_format_setting_key, g_opengl_target_shader_format_setting_value},
            };

            return windows_target_settings;
        }

        shader_platform shader_format_to_shader_platform(const std::string& shader_format)
        {
            if (shader_format == g_name_d3d_sm5)    return shader_platform::D3D_SM5;
            if (shader_format == g_name_d3d_sm6)    return shader_platform::D3D_SM6;
            if (shader_format == g_name_gl_sm4_3)   return shader_platform::GL_SM4_3;

            CERA_ASSERT("Invalid shader format given: {}", shader_format);
            return shader_platform::NONE;
        }
    } // namespace renderer
} // namespace cera