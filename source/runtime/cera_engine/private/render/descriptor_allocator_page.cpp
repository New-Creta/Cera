#include "render/descriptor_allocator_page.h"
#include "render/device.h"
#include "render/d3dx12_call.h"

namespace cera
{
    descriptor_allocator_page::descriptor_allocator_page(device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptors)
        : m_device(device)
        , m_heap_type(type)
        , m_num_descriptors_in_heap(numDescriptors)
    {
        auto d3d_device = m_device.get_d3d_device();

        D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
        heap_desc.Type = m_heap_type;
        heap_desc.NumDescriptors = m_num_descriptors_in_heap;

        if (DX_FAILED(d3d_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_d3d12_descriptor_heap))))
        {
            assert(false && "Unable to CreateDescriptorHeap");
        }

        m_base_descriptor = m_d3d12_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
        m_descriptor_handle_increment_size = d3d_device->GetDescriptorHandleIncrementSize(m_heap_type);
        m_num_free_handles = m_num_descriptors_in_heap;

        // Initialize the free lists
        add_new_block(0, m_num_free_handles);
    }

    descriptor_allocator_page::~descriptor_allocator_page() = default;

    D3D12_DESCRIPTOR_HEAP_TYPE descriptor_allocator_page::get_heap_type() const
    {
        return m_heap_type;
    }

    bool descriptor_allocator_page::has_space(u32 numDescriptors) const
    {
        return m_free_list_by_size.lower_bound(numDescriptors) != m_free_list_by_size.end();
    }

    u32 descriptor_allocator_page::num_free_handles() const
    {
        return m_num_free_handles;
    }

    descriptor_allocation descriptor_allocator_page::allocate(u32 numDescriptors)
    {
        std::lock_guard<std::mutex> lock(m_allocation_mutex);

        // There are less than the requested number of descriptors left in the heap.
        // Return a NULL descriptor and try another heap.
        if (numDescriptors > m_num_free_handles)
        {
            return descriptor_allocation();
        }

        // Get the first block that is large enough to satisfy the request.
        auto smallest_block_it = m_free_list_by_size.lower_bound(numDescriptors);
        if (smallest_block_it == m_free_list_by_size.end())
        {
            // There was no free block that could satisfy the request.
            return descriptor_allocation();
        }

        // The size of the smallest block that satisfies the request.
        auto block_size = smallest_block_it->first;

        // The pointer to the same entry in the FreeListByOffset map.
        auto offset_it = smallest_block_it->second;

        // The offset in the descriptor heap.
        auto offset = offset_it->first;

        // Remove the existing free block from the free list.
        m_free_list_by_size.erase(smallest_block_it);
        m_free_list_by_offset.erase(offset_it);

        // Compute the new free block that results from splitting this block.
        auto new_offset = offset + numDescriptors;
        auto new_size = block_size - numDescriptors;

        if (new_size > 0)
        {
            // If the allocation didn't exactly match the requested size,
            // return the left-over to the free list.
            add_new_block(new_offset, new_size);
        }

        // Decrement free handles.
        m_num_free_handles -= numDescriptors;

        return descriptor_allocation(
            CD3DX12_CPU_DESCRIPTOR_HANDLE(m_base_descriptor, offset, m_descriptor_handle_increment_size),
            numDescriptors, m_descriptor_handle_increment_size, shared_from_this());
    }

    void descriptor_allocator_page::free(descriptor_allocation&& descriptorHandle)
    {
        // Compute the offset of the descriptor within the descriptor heap.
        auto offset = compute_offset(descriptorHandle.get_descriptor_handle());

        std::lock_guard<std::mutex> lock(m_allocation_mutex);

        // Don't add the block directly to the free list until the frame has completed.
        m_stale_descriptors.emplace(offset, descriptorHandle.get_num_handles());
    }

    void descriptor_allocator_page::release_stale_descriptors()
    {
        std::lock_guard<std::mutex> lock(m_allocation_mutex);

        while (!m_stale_descriptors.empty() )
        {
            auto& stale_descriptor = m_stale_descriptors.front();

            // The offset of the descriptor in the heap.
            auto offset = stale_descriptor.offset;
            // The number of descriptors that were allocated.
            auto num_descriptors = stale_descriptor.size;

            free_block(offset, num_descriptors);

            m_stale_descriptors.pop();
        }
    }

    u32 descriptor_allocator_page::compute_offset(D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        return static_cast<uint32_t>(handle.ptr - m_base_descriptor.ptr) / m_descriptor_handle_increment_size;
    }

    void descriptor_allocator_page::add_new_block(u32 offset, u32 numDescriptors)
    {
        auto offset_it = m_free_list_by_offset.emplace(offset, numDescriptors);
        auto sizeIt = m_free_list_by_size.emplace(numDescriptors, offset_it.first);
        offset_it.first->second.FreeListBySizeIt = sizeIt;
    }

    void descriptor_allocator_page::free_block(u32 offset, u32 numDescriptors)
    {
        // Find the first element whose offset is greater than the specified offset.
        // This is the block that should appear after the block that is being freed.
        auto next_block_it = m_free_list_by_offset.upper_bound(offset);

        // Find the block that appears before the block being freed.
        auto prev_block_it = next_block_it;
        // If it's not the first block in the list.
        if (prev_block_it != m_free_list_by_offset.begin())
        {
            // Go to the previous block in the list.
            --prev_block_it;
        }
        else
        {
            // Otherwise, just set it to the end of the list to indicate that no
            // block comes before the one being freed.
            prev_block_it = m_free_list_by_offset.end();
        }

        // Add the number of free handles back to the heap.
        // This needs to be done before merging any blocks since merging
        // blocks modifies the numDescriptors variable.
        m_num_free_handles += numDescriptors;

        if (prev_block_it != m_free_list_by_offset.end() &&
            offset == prev_block_it->first + prev_block_it->second.Size)
        {
            // The previous block is exactly behind the block that is to be freed.
            //
            // PrevBlock.Offset           Offset
            // |                          |
            // |<-----PrevBlock.Size----->|<------Size-------->|
            //

            // Increase the block size by the size of merging with the previous block.
            offset = prev_block_it->first;
            numDescriptors += prev_block_it->second.Size;

            // Remove the previous block from the free list.
            m_free_list_by_size.erase(prev_block_it->second.FreeListBySizeIt);
            m_free_list_by_offset.erase(prev_block_it);
        }

        if (next_block_it != m_free_list_by_offset.end() &&
            offset + numDescriptors == next_block_it->first)
        {
            // The next block is exactly in front of the block that is to be freed.
            //
            // Offset               NextBlock.Offset 
            // |                    |
            // |<------Size-------->|<-----NextBlock.Size----->|

            // Increase the block size by the size of merging with the next block.
            numDescriptors += next_block_it->second.Size;

            // Remove the next block from the free list.
            m_free_list_by_size.erase(next_block_it->second.FreeListBySizeIt);
            m_free_list_by_offset.erase(next_block_it);
        }

        // Add the freed block to the free list.
        add_new_block(offset, numDescriptors);
    }
}