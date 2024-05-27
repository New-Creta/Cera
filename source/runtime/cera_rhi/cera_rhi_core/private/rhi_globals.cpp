#include "rhi_globals.h"

#include "common/rhi_shader_format_names.h"

namespace cera
{
    namespace renderer
    {
        /** True if the render hardware has been initialized. */
        bool g_rhi_initialized = false;
        /** Force disable shader model 6 */
        bool g_force_disable_sm6 = false;
        /** Use the warp adapter if specified */
        bool g_use_warp_adapter = false;
        /** Allow software rasterization */
        bool g_allow_software_rendering = false;
        /** Use debug layer */
        bool g_is_debug_layer_enabled = _DEBUG;
        /** Use GPU validation */
        bool g_with_gpu_validation = _DEBUG && true;

        /** Name of the preferred rendering hardware interface */
        std::string g_preferred_rhi_name = "dx12";
        /** Name of the preferred feature level */
        std::string g_preferred_rhi_shader_format_name = shader_formats::g_name_d3d_sm6;

        /** The maximum feature level supported on this machine */
        feature_level g_max_feature_level = feature_level::D3D_SM5;
        /** The current shader platform */
        shader_platform g_max_shader_platform = shader_platform::D3D_SM5;

        /** Table for finding out which shader platform corresponds to a given feature level for this RHI. */
        shader_platform g_shader_platform[(s32)feature_level::NUM];

        namespace conversions
        {
            //-------------------------------------------------------------------------
            vendor to_gpu_vendor_id(u32 in_vendor_id)
            {
                switch ((vendor)in_vendor_id)
                {
                case vendor::NOTQUERIED:
                    return vendor::NOTQUERIED;

                case vendor::AMD:
                case vendor::IMGTEC:
                case vendor::NVIDIA:
                case vendor::ARM:
                case vendor::BROADCOM:
                case vendor::QUALCOMM:
                case vendor::INTEL:
                case vendor::SAMSUNGAMD:
                case vendor::MICROSOFT:
                    return (vendor)in_vendor_id;

                default:
                    break;
                }

                return vendor::UNKNOWN;
            }
        } // namespace conversions
    }
}