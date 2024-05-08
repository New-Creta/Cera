#include "descriptors/descriptor_allocation.h"

#include "descriptors/descriptor_allocator_page.h"

#include "cera_engine/diagnostics/assert.h"

namespace cera
{
  namespace renderer
  {
    DescriptorAllocation::DescriptorAllocation()
        : m_descriptor {0}
        , m_num_handles(0)
        , m_descriptor_size(0)
        , m_page(nullptr)
    {
    }

    DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, u32 numHandles, std::memory_size descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page)
        : m_descriptor(descriptor)
        , m_num_handles(numHandles)
        , m_descriptor_size(descriptorSize)
        , m_page(page)
    {
    }

    DescriptorAllocation::~DescriptorAllocation()
    {
      free();
    }

    DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& allocation)
        : m_descriptor(std::exchange(allocation.m_descriptor, {0}))
        , m_num_handles(std::exchange(allocation.m_num_handles, 0))
        , m_descriptor_size(std::exchange(allocation.m_descriptor_size, 0))
        , m_page(std::exchange(allocation.m_page, nullptr))
    {
    }

    DescriptorAllocation& DescriptorAllocation::operator=(DescriptorAllocation&& other)
    {
      // Free this descriptor if it points to anything.
      free();

      m_descriptor      = std::exchange(other.m_descriptor, {0});
      m_num_handles     = std::exchange(other.m_num_handles, 0);
      m_descriptor_size = std::exchange(other.m_descriptor_size, 0);
      m_page            = std::exchange(other.m_page, nullptr);

      return *this;
    }

    bool DescriptorAllocation::is_null() const
    {
      return m_descriptor.ptr == 0;
    }

    bool DescriptorAllocation::is_valid() const
    {
      return !is_null();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::descriptor_handle(u32 offset) const
    {
      CERA_ASSERT_X(offset < m_num_handles, "Descriptor handle offset excees number of descriptor handles");

      return {m_descriptor.ptr + (m_descriptor_size * offset)};
    }

    u32 DescriptorAllocation::num_handles() const
    {
      return m_num_handles;
    }

    std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocation::descriptor_allocator_page() const
    {
      return m_page;
    }

    void DescriptorAllocation::free()
    {
      if(!is_null() && m_page)
      {
        m_page->free(std::move(*this));

        m_descriptor.ptr  = 0;
        m_num_handles     = 0;
        m_descriptor_size = 0;
        m_page.reset();
      }
    }
  } // namespace renderer
} // namespace cera