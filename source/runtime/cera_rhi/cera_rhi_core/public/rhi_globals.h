#pragma once

#include "util/types.h"

#include "common/rhi_feature_level.h"
#include "common/rhi_shader_platform.h"

#include <string>

namespace cera
{
    namespace renderer
    {
        /** True if the render hardware has been initialized. */
        extern bool g_rhi_initialized;
        /** Name of the preferred rendering hardware interface */
        extern std::string g_preferred_rhi_name;
        /** Name of the preferred feature level */
        extern std::string g_preferred_rhi_shader_format_name;

        /** Table for finding out which shader platform corresponds to a given feature level for this RHI. */
        extern shader_platform g_shader_platform[g_num_feature_levels];
    }
}