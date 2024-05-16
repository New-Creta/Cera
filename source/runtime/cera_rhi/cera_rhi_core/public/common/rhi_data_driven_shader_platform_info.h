#pragma once

#include "util/types.h"

#include "common/rhi_feature_level.h"
#include "common/rhi_shader_platform.h"

#include <string>
#include <array>

namespace cera
{
    namespace renderer
    {
        struct data_driven_shader_platform_settings;

        class data_driven_shader_platform_info
        {
          public:
            static void                 initialize();

            static const std::string&   get_name(shader_platform platform);
            static const std::string&   get_shader_format(shader_platform platform);

            static const bool           is_language_d3d(shader_platform platform);
            static const bool           is_language_ogl(shader_platform platform);
            static const bool           is_supports_msaa(shader_platform platform);
            
            static const feature_level  get_max_feature_level(shader_platform platform);

        private:
            static std::array<data_driven_shader_platform_info, g_num_supported_shader_platform> s_infos;

            static void parse_data_driven_shader_platform_settings(const data_driven_shader_platform_settings& settings);

          private:
            data_driven_shader_platform_info();

            std::string m_name;
            std::string m_language;
            std::string m_shader_format;

            feature_level m_max_feature_level;

            s32 m_supports_msaa;
        };
    } // namespace renderer
} // namespace cera