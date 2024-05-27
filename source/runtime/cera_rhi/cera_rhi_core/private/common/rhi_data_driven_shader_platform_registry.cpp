#include "common/rhi_data_driven_shader_platform_registry.h"

#include "common/rhi_feature_level.h"
#include "common/rhi_shader_format_names.h"
#include "common/rhi_shader_language_names.h"

namespace cera
{
    namespace renderer
    {
        namespace config
        {
            data_driven_shader_platform_settings load_d3d_sm5_shader_platform_data()
            {
                data_driven_shader_platform_settings info;

                info.name               = "Direct 3D Shader Model 5";
                
                info.language           = shader_languages::g_name_d3d_language;
                info.shader_format      = shader_formats::g_name_d3d_sm5;
                info.shader_platform    = shader_platform::D3D_SM5;

                info.max_feature_level  = feature_level::D3D_SM5;

                info.supports_msaa      = 1;
                info.supports_bindless  = 0;

                return info;
            }

            data_driven_shader_platform_settings load_d3d_sm6_shader_platform_data()
            {
                data_driven_shader_platform_settings info;

                info.name               = "Direct 3D Shader Model 6";
                
                info.language           = shader_languages::g_name_d3d_language;
                info.shader_format      = shader_formats::g_name_d3d_sm6;
                info.shader_platform    = shader_platform::D3D_SM6;

                info.max_feature_level  = feature_level::D3D_SM6;

                info.supports_msaa      = 1;
                info.supports_bindless  = 1;

                return info;
            }

            data_driven_shader_platform_settings load_gl_sm4_3_shader_platform_data()
            {
                data_driven_shader_platform_settings info;

                // OpenGL shader model 4.3 is equal to DirectX shader model 5
                info.name               = "OpenGL Shader Model 4.3";

                info.language           = shader_languages::g_name_ogl_language;
                info.shader_format      = shader_formats::g_name_ogl_sm5;
                info.shader_platform    = shader_platform::OGL_SM5;

                info.max_feature_level  = feature_level::OGL_SM5;

                info.supports_msaa      = 1;
                info.supports_bindless  = 0;

                return info;
            }
        } // namespace config

        namespace data_driven_shader_platform_registry
        {
            std::vector<data_driven_shader_platform_settings> load_all_data_driven_shader_platform_settings()
            {
                return 
                {
                    config::load_d3d_sm5_shader_platform_data(),
                    config::load_d3d_sm6_shader_platform_data(),
                    config::load_gl_sm4_3_shader_platform_data(),
                };
            }
        };
    } // namespace renderer
} // namespace cera