#pragma once

#include "util/types.h"
#include "util/windows_types.h"
#include "util/memory_size.h"

#include "directx_util.h"

#include "resources/rhi_resource.h"

#include <memory>
#include <queue>
#include <deque>

namespace cera
{
    namespace renderer
    {
        class d3d12_device;

        class UploadBuffer : public rhi_resource
        {
        public:
            RESOURCE_CLASS_TYPE(UploadBuffer);

            // Used to upload data to the GPU
            struct Allocation
            {
                void* CPU;
                D3D12_GPU_VIRTUAL_ADDRESS GPU;
            };

            /**
            * The maximum size of an allocation is the size of a single page.
            */
            s64 page_size() const;

            /**
             * Allocate memory in an Upload heap.
             * An allocation must not exceed the size of a page.
             * Use a memcpy or similar method to copy the
             * buffer data to CPU pointer in the Allocation structure returned from
             * this function.
             */
            Allocation allocate(memory_size sizeInBytes, s64 alignment);

            /**
             * Release all allocated pages. This should only be done when the command list
             * is finished executing on the CommandQueue.
             */
            void reset();

        protected:
            friend struct std::default_delete<UploadBuffer>;

            /**
            * @param pageSize The size to use to allocate new pages in GPU memory.
            */
            explicit UploadBuffer(d3d12_device& device, memory_size pageSize = 2_mb);
            virtual ~UploadBuffer();

        private:
            // A single page for the allocator.
            class Page
            {
            public:
                Page(d3d12_device& device, memory_size sizeInBytes);
                ~Page();

                // Check to see if the page has room to satisfy the requested
                // allocation.
                bool has_space(memory_size sizeInBytes, s64 alignment) const;

                // Allocate memory from the page.
                // Throws std::bad_alloc if the the allocation size is larger
                // that the page size or the size of the allocation exceeds the 
                // remaining space in the page.
                Allocation allocate(memory_size sizeInBytes, s64 alignment);

                // reset the page for reuse.
                void reset();
            private:
                d3d12_device& m_device;
                wrl::com_ptr<ID3D12Resource> m_d3d12_resource;

                // Base pointer.
                void* m_CPU_ptr;
                D3D12_GPU_VIRTUAL_ADDRESS m_GPU_ptr;

                // Allocated page size.
                memory_size m_page_size;
                // Current allocation offset in bytes.
                s64 m_offset;
            };

        private:
            // Request a page from the pool of available pages
            // or create a new page if there are no available pages.
            std::shared_ptr<Page> request_page();

        private:
            // A pool of memory pages.
            using PagePool = std::deque< std::shared_ptr<Page>>;

            // The device that was used to create this upload buffer.
            d3d12_device& m_device;

            PagePool m_page_pool;
            PagePool m_available_pages;

            std::shared_ptr<Page> m_current_page;

            memory_size m_page_size;
        };
    }
}