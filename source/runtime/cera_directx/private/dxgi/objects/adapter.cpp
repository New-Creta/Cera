#include "dxgi/objects/adapter.h"
#include "dxgi/dxgi_util.h"

#include "gpu_description.h"

#include "util/log.h"
#include "util/types.h"
#include "util/memory_size.h"
#include "util/string_op.h"

#include "wrl/comobject.h"

#include <cstdlib>
#include <dxgi.h>
#include <string>

namespace
{
    const u32 g_adapter_description_size = 64u;

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
    template <typename dxgi_adapter_desc>
    cera::renderer::GpuDescription convert_description(const dxgi_adapter_desc& dxgi_desc)
    {
        cera::renderer::GpuDescription desc;

        desc.name = cera::string_operations::to_multibyte(dxgi_desc.Description, g_adapter_description_size);
        desc.vendor_name = vendor_to_string(dxgi_desc.VendorId);

        desc.vendor_id = dxgi_desc.VendorId;
        desc.device_id = dxgi_desc.DeviceId;

        desc.dedicated_video_memory = cera::memory_size(dxgi_desc.DedicatedVideoMemory);
        desc.dedicated_system_memory = cera::memory_size(dxgi_desc.DedicatedSystemMemory);
        desc.shared_system_memory = cera::memory_size(dxgi_desc.SharedSystemMemory);

        return desc;
    }

    //-------------------------------------------------------------------------
    cera::renderer::GpuDescription get_description(const cera::wrl::ComPtr<IDXGIAdapter4>& adapter)
    {
        cera::renderer::GpuDescription desc;

        DXGI_ADAPTER_DESC1 dxgi_desc;
        adapter->GetDesc1(&dxgi_desc);
        desc = convert_description(dxgi_desc);

        return desc;
    }
} // namespace

namespace cera
{
    namespace dxgi
    {
        //-------------------------------------------------------------------------
        Adapter::Adapter(wrl::ComPtr<IDXGIAdapter4>&& adapter)
            : ComObject(std::move(adapter)), m_description(::get_description(com_ptr()))
        {
        }

        //-------------------------------------------------------------------------
        const renderer::GpuDescription& Adapter::description() const
        {
            return m_description;
        }
    } // namespace dxgi
} // namespace cera
