#pragma once

#include "util/types.h"

#include <array>
#include <optional>

namespace cera
{
    namespace renderer
    {
        enum class shader_platform;
        enum class feature_level;

        enum class windows_rhi_type
        {
            D3D11,
            D3D12,
            OPENGL,
            NUM
        };

        static constexpr s32 g_num_windows_rhi_types = static_cast<s32>(windows_rhi_type::NUM);

        struct windows_rhi
        {
            std::vector<shader_platform> shader_platforms;
            std::vector<feature_level> feature_levels;
        };

        struct windows_rhi_config
        {
            std::optional<windows_rhi_type> default_rhi;

            std::array<windows_rhi, g_num_windows_rhi_types> rhi_configs;

            bool is_empty() const;
            bool is_rhi_supported(windows_rhi_type type) const;
            bool is_feature_level_targted(windows_rhi_type type, feature_level in_feature_level) const;

            std::optional<feature_level> get_highest_supported_feature_level(windows_rhi_type in_windows_rhi) const;
            std::optional<feature_level> get_next_highest_supported_feature_level(windows_rhi_type in_windows_rhi, feature_level in_feature_level) const;

            std::optional<windows_rhi_type> get_first_rhi_with_feature_level_support(feature_level in_feature_level) const;
        };

        extern std::array<windows_rhi_type, 3> g_search_order;
    }
}