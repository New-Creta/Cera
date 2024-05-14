#include "rhi_globals.h"

namespace cera
{
    namespace renderer
    {
        /** True if the render hardware has been initialized. */
        bool g_rhi_initialized = false;

        /** Table for finding out which shader platform corresponds to a given feature level for this RHI. */
        shader_platform g_shader_platform[feature_level::NUM];
    }
}