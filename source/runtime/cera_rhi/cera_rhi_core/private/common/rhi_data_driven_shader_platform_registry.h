#pragma once

#include "common/rhi_shader_platform.h"
#include "common/rhi_feature_level.h"

#include <string>
#include <vector>

namespace cera
{
    namespace renderer
    {
        /**
         * This needs to be converted to a settings file in the end
         */
        struct data_driven_shader_platform_settings
        {
            std::string name;
            std::string language;
            std::string shader_format;

            shader_platform shader_platform;
            feature_level max_feature_level;

            s32 supports_msaa;
        };

        namespace data_driven_shader_platform_registry
        {
            std::vector<data_driven_shader_platform_settings> load_all_data_driven_shader_platform_settings();
        };
    }
} // namespace cera