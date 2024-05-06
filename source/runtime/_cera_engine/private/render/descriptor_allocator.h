#pragma once

#include "util/types.h"

#include "render/d3dx12_declarations.h"
#include "render/descriptor_allocation.h"

#include <cstdint>
#include <mutex>
#include <memory>
#include <set>
#include <vector>

namespace cera
{
    class device;
    class descriptor_allocator_page;

    /*
    * The DescriptorAllocator class is used to allocate descriptors to the application when loading new resources (like textures). 
    * In a typical game engine, resources may need to be loaded and unloaded from memory at sporadic moments while the player moves around the level.
    * To support large dynamic worlds, it may be necessary to initially load some resources, unload them from memory, and reload different resources.
    * The DescriptorAllocator manages all of the descriptors that are required to describe those resources. 
    * Descriptors that are no longer used (for example, when a resource is unloaded from memory) will be automatically returned back to the descriptor heap for reuse.
    */
    class descriptor_allocator
    {
    public:
        /**
         * Allocate a number of contiguous descriptors from a CPU visible descriptor heap.
         *
         * @param numDescriptors The number of contiguous descriptors to allocate.
         * Cannot be more than the number of descriptors per descriptor heap.
         */
        descriptor_allocation allocate(u32 numDescriptors = 1);

        /**
         * When the frame has completed, the stale descriptors can be released.
         */
        void release_stale_descriptors();

    protected:
        friend class std::default_delete<descriptor_allocator>;

        // Can only be created by the Device.
        descriptor_allocator(device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptorsPerHeap = 256);
        virtual ~descriptor_allocator();

    private:        
        // Create a new heap with a specific number of descriptors.
        std::shared_ptr<descriptor_allocator_page> create_allocator_page();

    private:
        using descriptor_heap_pool = std::vector<std::shared_ptr<descriptor_allocator_page>>;

        device& m_device;
        D3D12_DESCRIPTOR_HEAP_TYPE m_heap_type;
        u32 m_num_descriptors_per_heap;

        descriptor_heap_pool m_heap_pool;
        // Indices of available heaps in the heap pool.
        std::set<size_t> m_available_heaps;

        std::mutex m_allocation_mutex;
    };
}