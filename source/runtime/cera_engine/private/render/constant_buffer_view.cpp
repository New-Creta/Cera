#include "render/constant_buffer_view.h"
#include "render/constant_buffer.h"
#include "render/device.h"
#include "util/memory_helpers.h"

namespace cera
{
    constant_buffer_view::constant_buffer_view(device& device, const std::shared_ptr<constant_buffer>& constantBuffer, size_t offset)
        : m_device(device)
        , m_constant_buffer(constantBuffer)
    {
        assert(constantBuffer);

        auto d3d_device = m_device.get_d3d_device();
        auto d3d_resource = m_constant_buffer->get_d3d_resource();

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
        cbv.BufferLocation = d3d_resource->GetGPUVirtualAddress() + offset;
        cbv.SizeInBytes = memory::align_up(m_constant_buffer->get_size_in_bytes(), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);  // Constant buffers must be aligned for hardware requirements.

        m_descriptor = device.allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        d3d_device->CreateConstantBufferView(&cbv, m_descriptor.get_descriptor_handle());
    }

    constant_buffer_view::~constant_buffer_view() = default;

    std::shared_ptr<constant_buffer> constant_buffer_view::get_constant_buffer() const
    {
        return m_constant_buffer;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE constant_buffer_view::get_descriptor_handle()
    {
        return m_descriptor.get_descriptor_handle();
    }
}