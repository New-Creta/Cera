#include "rhi_globals.h"

#include "common/rhi_shader_format_names.h"

namespace cera
{
    namespace renderer
    {
        /** True if the render hardware has been initialized. */
        bool g_rhi_initialized = false;
        /** Name of the preferred rendering hardware interface */
        std::string g_preferred_rhi_name = "dx12";
        /** Name of the preferred feature level */
        std::string g_preferred_rhi_shader_format_name = shader_formats::g_name_d3d_sm6;

        /** Table for finding out which shader platform corresponds to a given feature level for this RHI. */
        shader_platform g_shader_platform[(s32)feature_level::NUM];
    }
}