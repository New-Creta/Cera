#pragma once

#include "util/types.h"

#include "render/d3dx12_declarations.h"

#include "device/windows_types.h"

#include <cstdint>
#include <memory>
#include <queue>
#include <functional>

namespace cera
{
    class command_list;
    class root_signature;
    class device;

    class dynamic_descriptor_heap
    {
    public:
        dynamic_descriptor_heap(device& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 numDescriptorsPerHeap = 1024);
        ~dynamic_descriptor_heap();

        /**
         * Stages a contiguous range of CPU visible descriptors.
         * Descriptors are not copied to the GPU visible descriptor heap until
         * the CommitStagedDescriptors function is called.
         */
        void stage_descriptors(u32 rootParameterIndex, u32 offset, u32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptor);

        /**
         * Stage an inline CBV descriptor.
         */
        void stage_inline_CBV(u32 rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);

        /**
         * Stage an inline SRV descriptor.
         */
        void stage_inline_SRV(u32 rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);

        /**
         * Stage an inline UAV descriptor.
         */
        void stage_inline_UAV(u32 rootParamterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);

        /**
         * Copy all of the staged descriptors to the GPU visible descriptor heap and
         * bind the descriptor heap and the descriptor tables to the command list.
         * The passed-in function object is used to set the GPU visible descriptors
         * on the command list. Two possible functions are:
         *   * Before a draw    : ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
         *   * Before a dispatch: ID3D12GraphicsCommandList::SetComputeRootDescriptorTable
         *
         * Since the DynamicDescriptorHeap can't know which function will be used, it must
         * be passed as an argument to the function.
         */
        void commit_staged_descriptors_for_draw(command_list& commandList);
        void commit_staged_descriptors_for_dispatch(command_list& commandList);

        /**
         * Copies a single CPU visible descriptor to a GPU visible descriptor heap.
         * This is useful for the
         *   * ID3D12GraphicsCommandList::ClearUnorderedAccessViewFloat
         *   * ID3D12GraphicsCommandList::ClearUnorderedAccessViewUint
         * methods which require both a CPU and GPU visible descriptors for a UAV
         * resource.
         *
         * @param commandList The command list is required in case the GPU visible
         * descriptor heap needs to be updated on the command list.
         * @param cpuDescriptor The CPU descriptor to copy into a GPU visible
         * descriptor heap.
         *
         * @return The GPU visible descriptor.
         */
        D3D12_GPU_DESCRIPTOR_HANDLE copy_descriptor(command_list& comandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);

        /**
         * Parse the root signature to determine which root parameters contain
         * descriptor tables and determine the number of descriptors needed for
         * each table.
         */
        void parse_root_signature(const std::shared_ptr<root_signature>& rootSignature);

        /**
         * Reset used descriptors. This should only be done if any descriptors
         * that are being referenced by a command list has finished executing on the
         * command queue.
         */
        void reset();

    private:
        // Request a descriptor heap if one is available.
        wrl::ComPtr<ID3D12DescriptorHeap> request_descriptor_heap();
        // Create a new descriptor heap of no descriptor heap is available.
        wrl::ComPtr<ID3D12DescriptorHeap> create_descriptor_heap();

        // Compute the number of stale descriptors that need to be copied
        // to GPU visible descriptor heap.
        u32 compute_stale_descriptor_count() const;

        /**
         * Copy all of the staged descriptors to the GPU visible descriptor heap and
         * bind the descriptor heap and the descriptor tables to the command list.
         * The passed-in function object is used to set the GPU visible descriptors
         * on the command list. Two possible functions are:
         *   * Before a draw    : ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
         *   * Before a dispatch: ID3D12GraphicsCommandList::SetComputeRootDescriptorTable
         *
         * Since the DynamicDescriptorHeap can't know which function will be used, it must
         * be passed as an argument to the function.
         */
        void commit_descriptor_tables(command_list& commandList, std::function<void(ID3D12GraphicsCommandList*, u32, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);
        void commit_inline_descriptors(command_list& commandList, const D3D12_GPU_VIRTUAL_ADDRESS* bufferLocations, u32& bitMask, std::function<void(ID3D12GraphicsCommandList*, u32, D3D12_GPU_VIRTUAL_ADDRESS)> setFunc);

    private:
        /**
        * The maximum number of descriptor tables per root signature.
        * A 32-bit mask is used to keep track of the root parameter indices that
        * are descriptor tables.
        */
        static const u32 s_max_descriptor_tables = 32;

    private:
        /**
         * A structure that represents a descriptor table entry in the root signature.
         */
        struct descriptor_table_cache
        {
            descriptor_table_cache()
                : num_descriptors(0)
                , base_descriptor(nullptr)
            {}

            // Reset the table cache.
            void reset()
            {
                num_descriptors = 0;
                base_descriptor = nullptr;
            }

            // The number of descriptors in this descriptor table.
            u32 num_descriptors;
            // The pointer to the descriptor in the descriptor handle cache.
            D3D12_CPU_DESCRIPTOR_HANDLE* base_descriptor;
        };

        // The device that is used to create this descriptor heap.
        device& m_device;

        // Describes the type of descriptors that can be staged using this 
        // dynamic descriptor heap.
        // Valid values are:
        //   * D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
        //   * D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
        // This parameter also determines the type of GPU visible descriptor heap to 
        // create.
        D3D12_DESCRIPTOR_HEAP_TYPE m_descriptor_heap_type;

        // The number of descriptors to allocate in new GPU visible descriptor heaps.
        u32 m_num_descriptors_per_heap;

        // The increment size of a descriptor.
        u32 m_descriptor_handle_increment_size;

        // The descriptor handle cache.
        std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> m_descriptor_handle_cache;

        // Descriptor handle cache per descriptor table.
        descriptor_table_cache m_descriptor_table_cache[s_max_descriptor_tables];

        // Inline CBV
        D3D12_GPU_VIRTUAL_ADDRESS m_inline_CBV[s_max_descriptor_tables];
        // Inline SRV
        D3D12_GPU_VIRTUAL_ADDRESS m_inline_SRV[s_max_descriptor_tables];
        // Inline UAV
        D3D12_GPU_VIRTUAL_ADDRESS m_inline_UAV[s_max_descriptor_tables];

        // Each bit in the bit mask represents the index in the root signature
        // that contains a descriptor table.    
        u32 m_descriptor_table_bit_mask;

        // Each bit set in the bit mask represents a descriptor table
        // in the root signature that has changed since the last time the 
        // descriptors were copied.
        u32 m_stale_descriptor_table_bit_mask;
        u32 m_stale_CBV_bit_mask;
        u32 m_stale_SRV_bit_mask;
        u32 m_stale_UAV_bit_mask;

        using descriptor_heap_pool = std::queue< wrl::ComPtr<ID3D12DescriptorHeap> >;

        descriptor_heap_pool m_descriptor_heap_pool;
        descriptor_heap_pool m_available_descriptor_heaps;

        wrl::ComPtr<ID3D12DescriptorHeap> m_current_descriptor_heap;
        CD3DX12_GPU_DESCRIPTOR_HANDLE m_current_GPU_descriptor_handle;
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_current_CPU_descriptor_handle;

        u32 m_num_free_handles;
    };
}