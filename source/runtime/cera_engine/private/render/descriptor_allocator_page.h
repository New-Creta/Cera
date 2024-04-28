#pragma once

#include "render/d3dx12_declarations.h"
#include "render/descriptor_allocation.h"

#include "device/windows_types.h"

#include <map>
#include <memory>
#include <mutex>
#include <queue>

namespace cera
{
    class device;

    /*
    * @brief A descriptor heap (page for the descriptor_allocator class).
    *
    * Variable sized memory allocation strategy based on:
    * http://diligentgraphics.com/diligent-engine/architecture/d3d12/variable-size-memory-allocations-manager/
    * 
    */
    class descriptor_allocator_page : public std::enable_shared_from_this<descriptor_allocator_page>
    {
    public:
        D3D12_DESCRIPTOR_HEAP_TYPE get_heap_type() const;

        /**
         * Check to see if this descriptor page has a contiguous block of descriptors
         * large enough to satisfy the request.
         */
        bool has_space(u32 numDescriptors) const;

        /**
         * Get the number of available handles in the heap.
         */
        u32 num_free_handles() const;

        /**
         * Allocate a number of descriptors from this descriptor heap.
         * If the allocation cannot be satisfied, then a NULL descriptor
         * is returned.
         */
        descriptor_allocation allocate(u32 numDescriptors);

        /**
         * Return a descriptor back to the heap.
         * @param frameNumber Stale descriptors are not freed directly, but put
         * on a stale allocations queue. Stale allocations are returned to the heap
         * using the DescriptorAllocatorPage::ReleaseStaleAllocations method.
         */
        void free(descriptor_allocation&& descriptorHandle);

        /**
         * Returned the stale descriptors back to the descriptor heap.
         */
        void release_stale_descriptors();

    protected:
        descriptor_allocator_page(device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptors);
        virtual ~descriptor_allocator_page();

    private:

        // Compute the offset of the descriptor handle from the start of the heap.
        u32 compute_offset(D3D12_CPU_DESCRIPTOR_HANDLE handle);

        // Adds a new block to the free list.
        void add_new_block(u32 offset, u32 numDescriptors);

        // Free a block of descriptors.
        // This will also merge free blocks in the free list to form larger blocks
        // that can be reused.
        void free_block(u32 offset, u32 numDescriptors);

    private:
        /*
        * The free_list_by_offset data structure stores a reference to the corresponding entry in the free_list_by_size map.
        * Similarly, each entry in the free_list_by_size map stores a reference by to the corresponding entry in the free_list_by_offset map.
        * 
        * This solution resembles a bi-directional map (Bimap in Boost)
        *   which provides optimized searching on both offset and size of each entry in the free list.
        */

        // The offset (in descriptors) within the descriptor heap.
        using offset_type = u32;
        // The number of descriptors that are available.
        using size_type = u32;

        struct free_block_info;

        // A map that lists the free blocks by the offset within the descriptor heap.
        using free_list_by_offset = std::map<offset_type, free_block_info>;
        // A map that lists the free blocks by size.
        // Needs to be a multimap since multiple blocks can have the same size.
        using free_list_by_size = std::multimap<size_type, free_list_by_offset::iterator>;

        struct free_block_info
        {
            free_block_info(size_type size)
                : Size(size)
            {}

            size_type Size;
            free_list_by_size::iterator FreeListBySizeIt;
        };

        free_list_by_offset m_free_list_by_offset;
        free_list_by_size m_free_list_by_size;

    private:
        struct stale_descriptor_info
        {
            stale_descriptor_info(offset_type offsetType, size_type sizeType)
                : offset(offsetType)
                , size(sizeType)
            {}

            // The offset within the descriptor heap.
            offset_type offset;
            // The number of descriptors
            size_type size;
        };

        // Stale descriptors are queued for release until the frame that they were freed
        // has completed.
        using stale_descriptor_queue = std::queue<stale_descriptor_info>;

        stale_descriptor_queue m_stale_descriptors;

    private:
        device& m_device;
        wrl::ComPtr<ID3D12DescriptorHeap> m_d3d12_descriptor_heap;
        D3D12_DESCRIPTOR_HEAP_TYPE m_heap_type;
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_base_descriptor;
        u32 m_descriptor_handle_increment_size;
        u32 m_num_descriptors_in_heap;
        u32 m_num_free_handles;

        std::mutex m_allocation_mutex;

    };
}