#pragma once

#include "util/types.h"

#include "common/rhi_feature_level.h"
#include "common/rhi_shader_platform.h"

#include <string>

namespace cera
{
    namespace renderer
    {
        /**
         * The VendorId is a unique identifier assigned by the PCI-SIG (Peripheral Component Interconnect Special Interest
         * Group) to identify the manufacturer of a PCI device, including graphics adapters. The VendorId is a 16-bit
         * unsigned integer that is typically included in the PCI Configuration space of the device.
         */
        enum class vendor
        {
            UNKNOWN = 0xffffffff,
            NOTQUERIED = 0,

            AMD = 0x1002,
            IMGTEC = 0x1010,
            NVIDIA = 0x10DE,
            ARM = 0x13B5,
            BROADCOM = 0x14E4,
            QUALCOMM = 0x5143,
            INTEL = 0x8086,
            SAMSUNGAMD = 0x144D,
            MICROSOFT = 0x1414,
        };

        namespace conversions
        {
            vendor to_gpu_vendor_id(u32 in_vendor_id);
        }

        /**
         * only set if RHI has the information (after init of the RHI and only if RHI has that information, never changes after that)
         * e.g. "NVIDIA GeForce GTX 670"
         */
        struct gpu_info
        {
            std::wstring    adapter_name;

            vendor          adapter_vendor_id;

            u32             adapter_device_id;
            u32             adapter_revision;

            bool            adapter_is_integrated;
        };

        /** True if the render hardware has been initialized. */
        extern bool g_rhi_initialized;
        /** Force disable shader model 6 */
        extern bool g_force_disable_sm6;
        /** Use the warp adapter if specified */
        extern bool g_use_warp_adapter;
        /** Allow software rasterization */
        extern bool g_allow_software_rendering;
        /** Use debug layer */
        extern bool g_is_debug_layer_enabled;
        /** Use GPU validation */
        extern bool g_with_gpu_validation;

        /** Name of the preferred rendering hardware interface */
        extern std::string g_preferred_rhi_name;
        /** Name of the preferred feature level */
        extern std::string g_preferred_rhi_shader_format_name;

        /** Cached information about the selected GPU */
        extern gpu_info g_gpu_info;
        /** The maximum feature level supported on this machine */
        extern feature_level g_max_feature_level;
        /** The current shader platform */
        extern shader_platform g_max_shader_platform;

        /** Table for finding out which shader platform corresponds to a given feature level for this RHI. */
        extern shader_platform g_shader_platform[g_num_feature_levels];
    }
}