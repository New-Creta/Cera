#include "common/rhi_data_driven_shader_platform_info.h"
#include "common/rhi_data_driven_shader_platform_registry.h"
#include "common/rhi_data_driven_shader_platform_settings.h"
#include "common/rhi_shader_language_names.h"

#include "util/assert.h"

namespace cera
{
    namespace renderer
    {
        std::array<data_driven_shader_platform_info, g_num_supported_shader_platform> data_driven_shader_platform_info::s_infos;

        data_driven_shader_platform_info::data_driven_shader_platform_info()
        {
            std::memset(this, 0, sizeof(data_driven_shader_platform_info));

            m_max_feature_level = feature_level::NUM;

            m_supports_msaa = 1;
        }

        void data_driven_shader_platform_info::initialize()
        {
            static bool is_initialized = false;
            if (is_initialized)
            {
                return;
            }

            auto settings = data_driven_shader_platform_registry::load_all_data_driven_shader_platform_settings();
            for (auto& setting : settings)
            {
                // at this point, we can start pulling information out
                parse_data_driven_shader_platform_settings(setting);
            }

            is_initialized = true;
        }

        const std::string& data_driven_shader_platform_info::get_name(shader_platform platform)
        {
            CERA_ASSERT(s_infos.find(platform) != s_infos.cend());

            return s_infos[(s32)platform].m_name;
        }

        const std::string& data_driven_shader_platform_info::get_shader_format(shader_platform platform)
        {
            CERA_ASSERT(s_infos.find(platform) != s_infos.cend());

            return s_infos[(s32)platform].m_shader_format;
        }

        const bool data_driven_shader_platform_info::is_language_d3d(shader_platform platform)
        {
            CERA_ASSERT(s_infos.find(platform) != s_infos.cend());

            return s_infos[(s32)platform].m_language == shader_languages::g_name_d3d_language;
        }
        
        const bool data_driven_shader_platform_info::is_language_ogl(shader_platform platform)
        {
            CERA_ASSERT(s_infos.find(platform) != s_infos.cend());

            return s_infos[(s32)platform].m_language == shader_languages::g_name_ogl_language;
        }
        
        const bool data_driven_shader_platform_info::is_supports_msaa(shader_platform platform)
        {
            CERA_ASSERT(s_infos.find(platform) != s_infos.cend());

            return s_infos[(s32)platform].m_supports_msaa != 0;
        }

        const feature_level data_driven_shader_platform_info::get_max_feature_level(shader_platform platform)
        {
            CERA_ASSERT(s_infos.find(platform) != s_infos.cend());

            return s_infos[(s32)platform].m_max_feature_level;
        }

        void data_driven_shader_platform_info::parse_data_driven_shader_platform_settings(const data_driven_shader_platform_settings& settings)
        {
            data_driven_shader_platform_info& info = s_infos[settings.shader_platform];

            info.m_name                 = settings.name;
            info.m_language             = settings.language;
            info.m_shader_format        = settings.shader_format;
            info.m_max_feature_level    = settings.max_feature_level;
            info.m_supports_msaa        = settings.supports_msaa;
        }
    } // namespace renderer
} // namespace cera