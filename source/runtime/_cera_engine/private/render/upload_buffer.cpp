#include "render/upload_buffer.h"
#include "render/device.h"
#include "render/d3dx12_call.h"
#include "util/memory_helpers.h"

namespace cera
{
    upload_buffer::upload_buffer(device& device, size_t pageSize)
        :m_device(device)
        ,m_page_size(pageSize)
    {

    }

    upload_buffer::~upload_buffer() = default;

    size_t upload_buffer::get_page_size() const
    {
        return m_page_size;
    }

    upload_buffer::allocation upload_buffer::allocate(size_t sizeInBytes, size_t alignment)
    {
        assert(sizeInBytes <= m_page_size && "bad allocation");

        // If there is no current page, or the requested allocation exceeds the
        // remaining space in the current page, request a new page.
        if (!m_current_page || !m_current_page->has_space(sizeInBytes, alignment))
        {
            m_current_page = request_page();
        }

        return m_current_page->allocate(sizeInBytes, alignment);
    }

    void upload_buffer::reset()
    {
        m_current_page = nullptr;

        // Reset all available pages.
        m_available_pages = m_page_pool;

        for (auto page : m_available_pages)
        {
            // Reset the page for new allocations.
            page->reset();
        }
    }

    std::shared_ptr<upload_buffer::page> upload_buffer::request_page()
    {
        std::shared_ptr<page> new_page;

        if (!m_available_pages.empty())
        {
            new_page = m_available_pages.front();
            m_available_pages.pop_front();
        }
        else
        {
            new_page = std::make_shared<page>(m_device, m_page_size);
            m_page_pool.push_back(new_page);
        }

        return new_page;
    }

    upload_buffer::page::page(device& device, size_t sizeInBytes)
        :m_device(device)
        ,m_page_size(sizeInBytes)
        ,m_offset(0)
        ,m_CPU_ptr(nullptr)
        ,m_GPU_ptr(D3D12_GPU_VIRTUAL_ADDRESS(0))
    {
        auto d3d_device = m_device.get_d3d_device();

        if (DX_FAILED(d3d_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(m_page_size),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_d3d12_resource)
        )))
        {
            assert(false && "Failed to create UploadBuffer Page");
        }

        m_d3d12_resource->SetName(L"Upload Buffer (Page)");

        m_GPU_ptr = m_d3d12_resource->GetGPUVirtualAddress();
        m_d3d12_resource->Map(0, nullptr, &m_CPU_ptr);
    }

    upload_buffer::page::~page()
    {
        m_d3d12_resource->Unmap(0, nullptr);
        m_CPU_ptr = nullptr;
        m_GPU_ptr = D3D12_GPU_VIRTUAL_ADDRESS(0);
    }

    bool upload_buffer::page::has_space(size_t sizeInBytes, size_t alignment) const
    {
        size_t aligned_size = memory::align_up(sizeInBytes, alignment);
        size_t aligned_offset = memory::align_up(m_offset, alignment);

        return aligned_offset + aligned_size <= m_page_size;
    }

    upload_buffer::allocation upload_buffer::page::allocate(size_t sizeInBytes, size_t alignment)
    {
        // function is not thread safe! insert a std::lock_guard if thread safety is a requirement.
        // 
        // since UploadBuffer class instances are not used across multiple threads this is unnecessay 
        // overhead to incorperate this functionality

        assert(has_space(sizeInBytes, alignment) && "Can't allocator space from page");

        size_t aligned_size = memory::align_up(sizeInBytes, alignment);

        m_offset = memory::align_up(m_offset, alignment);

        upload_buffer::allocation allocation;
        allocation.CPU = static_cast<uint8_t*>(m_CPU_ptr) + m_offset;
        allocation.GPU = m_GPU_ptr + m_offset;

        m_offset += aligned_size;

        return allocation;
    }

    void upload_buffer::page::reset()
    {
        m_offset = 0;
    }
}