#include "rhi_directx_config.h"

#include "common/rhi_shader_platform.h"
#include "common/rhi_feature_level.h"

namespace cera
{
    namespace renderer
    {
        bool windows_rhi_config::is_empty()
        {
            for (const windows_rhi& rhi : rhi_configs)
            {
                if (!rhi.shader_platforms.empty())
                {
                    return false;
                }
            }

            return true;
        }

        bool windows_rhi_config::is_rhi_supported(windows_rhi_type type)
        {
            return !rhi_configs[(s32)type].shader_platforms.empty();
        }

        bool windows_rhi_config::is_feature_level_targted(windows_rhi_type type, feature_level in_feature_level)
        {
            for (feature_level supported_feature_level : rhi_configs[(s32)type].feature_levels)
            {
                if (supported_feature_level == in_feature_level)
                {
                    return true;
                }
            }
            return false;
        }

        std::optional<feature_level> windows_rhi_config::get_highest_supported_feature_level(windows_rhi_type in_windows_rhi)
        {
            const std::vector<feature_level>& feature_levels = rhi_configs[(s32)in_windows_rhi].feature_levels;
            if (feature_levels.empty())
            {
                return {};
            }

            feature_level max_feature_level = (feature_level)0;
            for (feature_level supported_feature_level : feature_levels)
            {
                max_feature_level = std::max(max_feature_level, supported_feature_level);
            }

            return max_feature_level;
        }

        std::optional<feature_level> windows_rhi_config::get_next_highest_supported_feature_level(windows_rhi_type in_windows_rhi, feature_level in_feature_level)
        {
            std::vector<feature_level> lower_feature_levels(rhi_configs[(s32)in_windows_rhi].feature_levels);

            lower_feature_levels.erase(std::remove_if(lower_feature_levels.begin(), lower_feature_levels.end(), 
                [in_feature_level](feature_level other_feature_level) 
                { 
                    return other_feature_level >= in_feature_level; 
                }), lower_feature_levels.end());

            if (!lower_feature_levels.empty())
            {
                feature_level max_feature_level = (feature_level)0;
                for (feature_level supported_feature_level : lower_feature_levels)
                {
                    max_feature_level = std::max(max_feature_level, supported_feature_level);
                }

                return max_feature_level;
            }

            return {};
        }

        std::optional<windows_rhi_type> windows_rhi_config::get_first_rhi_with_feature_level_support(feature_level in_feature_level)
        {
            static const const windows_rhi_type search_order[] = {
                windows_rhi_type::D3D12,
                windows_rhi_type::D3D11,
                windows_rhi_type::OPENGL,
            };

            for (windows_rhi_type windows_rhi_type : search_order)
            {
                if (is_feature_level_targted(windows_rhi_type, in_feature_level))
                {
                    return windows_rhi_type;
                }
            }

            return {};
        }
    }
}