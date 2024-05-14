#pragma once

#include "common/rhi_feature_level.h"
#include "common/rhi_shader_platform.h"

namespace cera
{
    namespace renderer
    {
        /** True if the render hardware has been initialized. */
        extern bool g_rhi_initialized;

        /** Table for finding out which shader platform corresponds to a given feature level for this RHI. */
        extern shader_platform g_shader_platform[g_num_feature_levels];
    }
}