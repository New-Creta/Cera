#include "render/descriptor_allocation.h"
#include "render/descriptor_allocator_page.h"

namespace cera
{
    descriptor_allocation::descriptor_allocation()
        : m_descriptor{ 0 }
        , m_num_handles(0)
        , m_descriptor_size(0)
        , m_page(nullptr)
    {

    }

    descriptor_allocation::descriptor_allocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, u32 numHandles, u32 descriptor_size, std::shared_ptr<descriptor_allocator_page> page)
        : m_descriptor(descriptor)
        , m_num_handles(numHandles)
        , m_descriptor_size(descriptor_size)
        , m_page(page)
    {

    }

    descriptor_allocation::~descriptor_allocation()
    {
        free();
    }

    descriptor_allocation::descriptor_allocation(descriptor_allocation&& allocation)
        : m_descriptor(std::exchange(allocation.m_descriptor, { 0 }))
        , m_num_handles(std::exchange(allocation.m_num_handles, 0))
        , m_descriptor_size(std::exchange(allocation.m_descriptor_size, 0))
        , m_page(std::exchange(allocation.m_page, nullptr))
    {

    }

    descriptor_allocation& descriptor_allocation::operator=(descriptor_allocation&& other)
    {
        // Free this descriptor if it points to anything.
        free();

        m_descriptor = std::exchange(other.m_descriptor, { 0 });
        m_num_handles = std::exchange(other.m_num_handles, 0);
        m_descriptor_size = std::exchange(other.m_descriptor_size, 0);
        m_page = std::exchange(other.m_page, nullptr);

        return *this;
    }

    bool descriptor_allocation::is_null() const
    {
        return m_descriptor.ptr == 0;
    }

    bool descriptor_allocation::is_valid() const
    {
        return !is_null();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE descriptor_allocation::get_descriptor_handle(u32 offset) const
    {
        assert(offset < m_num_handles);

        return { m_descriptor.ptr + (m_descriptor_size * offset) };
    }

    u32 descriptor_allocation::get_num_handles() const
    {
        return m_num_handles;
    }

    std::shared_ptr<descriptor_allocator_page> descriptor_allocation::get_descriptor_allocator_page() const
    {
        return m_page;
    }

    void descriptor_allocation::free()
    {
        if (!is_null() && m_page)
        {
            m_page->free(std::move(*this));

            m_descriptor.ptr = 0;
            m_num_handles = 0;
            m_descriptor_size = 0;
            m_page.reset();
        }
    }
}