#include "render/descriptor_allocator.h"
#include "render/descriptor_allocator_page.h"
#include "render/device.h"

#include "device/windows_declarations.h"

namespace cera
{
    namespace internal
    {
        struct make_allocator_page : public descriptor_allocator_page
        {
        public:
            make_allocator_page(device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptors)
                : descriptor_allocator_page(device, type, numDescriptors)
            {}

            ~make_allocator_page() override = default;
        };
    }

    descriptor_allocator::descriptor_allocator(device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptorsPerHeap)
        : m_device(device)
        , m_heap_type(type)
        , m_num_descriptors_per_heap(numDescriptorsPerHeap)
    {

    }

    descriptor_allocator::~descriptor_allocator()
    {

    }

    descriptor_allocation descriptor_allocator::allocate(u32 numDescriptors)
    {
        std::lock_guard<std::mutex> lock(m_allocation_mutex);

        descriptor_allocation allocation;

        for (auto iter = m_available_heaps.begin(); iter != m_available_heaps.end(); ++iter)
        {
            auto allocator_page = m_heap_pool[*iter];

            allocation = allocator_page->allocate(numDescriptors);

            if (allocator_page->num_free_handles() == 0)
            {
                iter = m_available_heaps.erase(iter);
            }

            // A valid allocation has been found.
            if (!allocation.is_null())
            {
                break;
            }
        }

        // No available heap could satisfy the requested number of descriptors.
        if (allocation.is_null())
        {
            m_num_descriptors_per_heap = std::max(m_num_descriptors_per_heap, numDescriptors);
            auto new_page = create_allocator_page();

            allocation = new_page->allocate(numDescriptors);
        }

        return allocation;
    }

    void descriptor_allocator::release_stale_descriptors()
    {
        std::lock_guard<std::mutex> lock(m_allocation_mutex);

        for (size_t i = 0; i < m_heap_pool.size(); ++i)
        {
            auto page = m_heap_pool[i];

            page->release_stale_descriptors();

            if (page->num_free_handles() > 0)
            {
                m_available_heaps.insert(i);
            }
        }
    }

    std::shared_ptr<descriptor_allocator_page> descriptor_allocator::create_allocator_page()
    {
        auto new_page = std::make_shared<internal::make_allocator_page>(m_device, m_heap_type, m_num_descriptors_per_heap);

        m_heap_pool.emplace_back(new_page);
        m_available_heaps.insert(m_heap_pool.size() - 1);

        return new_page;
    }
}