#include "gpu_helper.h"
#include "gpu_description.h"

#include "dxgi/dxgi_util.h"

#include "util/windows_types.h"
#include "util/string_op.h"

namespace cera
{
    namespace renderer
    {
        constexpr u32 g_adapter_description_size = 64u;

        //-------------------------------------------------------------------------
        /**
         * The VendorId is a unique identifier assigned by the PCI-SIG (Peripheral Component Interconnect Special Interest
         * Group) to identify the manufacturer of a PCI device, including graphics adapters. The VendorId is a 16-bit
         * unsigned integer that is typically included in the PCI Configuration space of the device.
         */
        enum class vendor
        {
            UNKNOWN = 0,
            AMD = 0x1002,
            NVIDIA = 0x10DE,
            INTEL = 0x163C
        };

        //-------------------------------------------------------------------------
        std::string vendor_to_string(s32 v)
        {
            // Enum reflection is not possible here as the integer values are
            // outside the valid range of values [0, 127] for this enumeration type
            switch (static_cast<vendor>(v))
            {
            case vendor::AMD:
                return std::string("AMD");
            case vendor::NVIDIA:
                return std::string("NVIDIA");
            case vendor::INTEL:
                return std::string("INTEL");
            default:
                return std::string("Unknown Vendor");
            }
        }

        //-------------------------------------------------------------------------
        renderer::GpuDescription convert_description(const DXGI_ADAPTER_DESC& dxgi_desc)
        {
            renderer::GpuDescription desc;

            desc.name = string_operations::to_multibyte(dxgi_desc.Description, g_adapter_description_size);
            desc.vendor_name = vendor_to_string(dxgi_desc.VendorId);

            desc.vendor_id = dxgi_desc.VendorId;
            desc.device_id = dxgi_desc.DeviceId;

            desc.dedicated_video_memory = memory_size(dxgi_desc.DedicatedVideoMemory);
            desc.dedicated_system_memory = memory_size(dxgi_desc.DedicatedSystemMemory);
            desc.shared_system_memory = memory_size(dxgi_desc.SharedSystemMemory);

            return desc;
        }

        GpuDescription find_best_gpu()
        {
            wrl::ComPtr<IDXGIFactory1> dxgi_factory;
            if (CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgi_factory) != S_OK || !dxgi_factory)
            {
                return GpuDescription();
            }

            DXGI_ADAPTER_DESC best_desc = {};
            wrl::ComPtr<IDXGIAdapter> temp_adapter;
            for (u32 adapter_index = 0; dxgi_factory->EnumAdapters(adapter_index, temp_adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++adapter_index)
            {
                if (temp_adapter)
                {
                    DXGI_ADAPTER_DESC desc;
                    temp_adapter->GetDesc(&desc);

                    if (desc.DedicatedVideoMemory > best_desc.DedicatedVideoMemory || adapter_index == 0)
                    {
                        best_desc = desc;
                    }
                }
            }

            return convert_description(best_desc);
        }
    }
}