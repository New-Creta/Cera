#include "dxgi/objects/adapter.h"
#include "dxgi/dxgi_util.h"

#include "gpu_description.h"

#include "util/log.h"
#include "util/types.h"

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
    std::string to_multibyte(const tchar* wide_character_buffer, size_t size)
    {
        std::string buffer;

        // Convert wide character string to multi byte character string.
        // size_t converted_chars => The amount of converted characters.
        // 0 terminate the string afterwards.
        size_t converted_chars = 0;
        auto result = wcstombs_s(&converted_chars, buffer.data(), size, wide_character_buffer, size);
        if (result != 0)
        {
            cera::log::error("Failed to convert wide character string to multi byte character string.");
            return std::string("Invalid String");
        }

        buffer.reset_null_termination_offset();

        return std::string(buffer.data(), static_cast<size_t>(converted_chars));
    }

    //-------------------------------------------------------------------------
    template <typename dxgi_adapter_desc>
    cera::renderer::GpuDescription convert_description(const dxgi_adapter_desc& dxgi_desc)
    {
        cera::renderer::GpuDescription desc;

        desc.name = to_multibyte(dxgiDesc.Description, g_adapter_description_size);
        desc.vendor_name = vendor_to_string(dxgiDesc.VendorId);

        desc.vendor_id = dxgiDesc.VendorId;
        desc.device_id = dxgiDesc.DeviceId;

        desc.dedicated_video_memory = memory_size(dxgiDesc.DedicatedVideoMemory);
        desc.dedicated_system_memory = memory_size(dxgiDesc.DedicatedSystemMemory);
        desc.shared_system_memory = memory_size(dxgiDesc.SharedSystemMemory);

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
