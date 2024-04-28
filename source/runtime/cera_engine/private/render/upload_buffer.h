#pragma once

#include "render/d3dx12_declarations.h"

#include "device/windows_types.h"

#include "util/types.h"
#include "util/memory_definitions.h"

#include <memory>
#include <deque>

namespace cera
{
    class device;

    class upload_buffer
    {
    public:
        // Used to upload data to the GPU
        struct allocation
        {
            void* CPU;
            D3D12_GPU_VIRTUAL_ADDRESS GPU;
        };

        /**
        * The maximum size of an allocation is the size of a single page.
        */
        size_t get_page_size() const;

        /**
         * Allocate memory in an Upload heap.
         * An allocation must not exceed the size of a page.
         * Use a memcpy or similar method to copy the
         * buffer data to CPU pointer in the Allocation structure returned from
         * this function.
         */
        allocation allocate(size_t sizeInBytes, size_t alignment);

        /**
         * Release all allocated pages. This should only be done when the command list
         * is finished executing on the CommandQueue.
         */
        void reset();

    protected:
        friend class std::default_delete<upload_buffer>;

        /**
        * @param pageSize The size to use to allocate new pages in GPU memory.
        */
        explicit upload_buffer(device& device, size_t pageSize = _2MB);
        virtual ~upload_buffer();

    private:
        // A single page for the allocator.
        class page
        {
        public:
            page(device& device, size_t sizeInBytes);
            ~page();

            // Check to see if the page has room to satisfy the requested
            // allocation.
            bool has_space(size_t sizeInBytes, size_t alignment) const;

            // Allocate memory from the page.
            // Throws std::bad_alloc if the the allocation size is larger
            // that the page size or the size of the allocation exceeds the 
            // remaining space in the page.
            allocation allocate(size_t sizeInBytes, size_t alignment);

            // reset the page for reuse.
            void reset();
        private:
            device& m_device;
            wrl::ComPtr<ID3D12Resource> m_d3d12_resource;

            // Base pointer.
            void* m_CPU_ptr;
            D3D12_GPU_VIRTUAL_ADDRESS m_GPU_ptr;

            // Allocated page size.
            size_t m_page_size;
            // Current allocation offset in bytes.
            size_t m_offset;
        };

    private:
        // Request a page from the pool of available pages
        // or create a new page if there are no available pages.
        std::shared_ptr<page> request_page();

    private:
        // A pool of memory pages.
        using page_pool = std::deque< std::shared_ptr<page>>;

        // The device that was used to create this upload buffer.
        device& m_device;

        page_pool m_page_pool;
        page_pool m_available_pages;

        std::shared_ptr<page> m_current_page;

        size_t m_page_size;
    };
}