#include "dxgi/objects/adapter.h"
#include "dxgi/log.h"
#include "gpu_description.h"
#include "rex_engine/diagnostics/logging/log_macros.h"
#include "rex_engine/engine/types.h"
#include "util/types.h"
#include "wrl/comobject.h"

#include <cstdlib>
#include <string>
#include <dxgi.h>

namespace
{
    const uint32 g_adapter_description_size = std::small_stack_string::max_size();

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
    auto vendor_to_string(s32 vendor) -> std::string
    {
        // Enum reflection is not possible here as the integer values are
        // outside the valid range of values [0, 127] for this enumeration type
        switch (static_cast<vendor>(vendor))
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
    std::small_stack_string to_multibyte(const tchar* wide_character_buffer, size_t size)
    {
        std::small_stack_string buffer;

        // Convert wide character string to multi byte character string.
        // size_t converted_chars => The amount of converted characters.
        // 0 terminate the string afterwards.
        size_t converted_chars = 0;
        auto result = wcstombs_s(&converted_chars, buffer.data(), size, wide_character_buffer, size);
        if (result != 0)
        {
            REX_ERROR(LogDXGI, "Failed to convert wide character string to multi byte character string.");
            return std::small_stack_string("Invalid String");
        }

        buffer.reset_null_termination_offset();

        return std::small_stack_string(
            buffer.data(), static_cast<size_t>(converted_chars)); // NOLINT(readability-redundant-string-cstr)
    }

    //-------------------------------------------------------------------------
    template <typename dxgi_adapter_desc>
    auto convert_description(const dxgi_adapter_desc& dxgi_desc) -> cera::renderer::GpuDescription
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
    auto get_description(const cera::wrl::ComPtr<IDXGIAdapter4>& adapter) -> cera::renderer::GpuDescription
    {
        cera::renderer::GpuDescription desc;

        DXGI_ADAPTER_DESC1 dxgi_desc;
        adapter->GetDesc1(&dxgi_desc);
        desc = convert_description(dxgi_desc);

        return desc;
    }
} // namespace


    namespace cera::dxgi
    {
        //-------------------------------------------------------------------------
        Adapter::Adapter(wrl::ComPtr<IDXGIAdapter4>&& adapter)
            : ComObject(std::move(adapter)), m_description(::get_description(com_ptr()))
        {
        }

        //-------------------------------------------------------------------------
        auto Adapter::description() const -> const renderer::GpuDescription&
        {
            return m_description;
        }
    } // namespace cera::dxgi

