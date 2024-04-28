#pragma once

#include "util/types.h"

#include "render/d3dx12_declarations.h"

#include <cstdint>
#include <memory>

namespace cera
{
    class descriptor_allocator_page;

    class descriptor_allocation
    {
    public:
        // Creates a NULL descriptor
        descriptor_allocation();
        descriptor_allocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, u32 numHandles, u32 descriptorSize, std::shared_ptr<descriptor_allocator_page> page);

        // The destructor will automatically free the allocation.
        ~descriptor_allocation();

        // Copies are not allowed.
        descriptor_allocation(const descriptor_allocation&) = delete;
        descriptor_allocation& operator=(const descriptor_allocation&) = delete;

        // Move is allowed.
        descriptor_allocation(descriptor_allocation&& allocation);
        descriptor_allocation& operator=(descriptor_allocation&& other);

        // Check if this a valid descriptor.
        bool is_null() const;
        bool is_valid() const;

        // Get a descriptor at a particular offset in the allocation.
        D3D12_CPU_DESCRIPTOR_HANDLE get_descriptor_handle(u32 offset = 0) const;

        // Get the number of (consecutive) handles for this allocation.
        u32 get_num_handles() const;

    private:
        // Get the heap that this allocation came from.
        // (For internal use only).
        std::shared_ptr<descriptor_allocator_page> get_descriptor_allocator_page() const;

        // Free the descriptor back to the heap it came from.
        void free();

    private:
        // The base descriptor.
        D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;
        // The number of descriptors in this allocation.
        uint32_t m_num_handles;
        // The offset to the next descriptor.
        uint32_t m_descriptor_size;

        // A pointer back to the original page where this allocation came from.
        std::shared_ptr<descriptor_allocator_page> m_page;
    };
}