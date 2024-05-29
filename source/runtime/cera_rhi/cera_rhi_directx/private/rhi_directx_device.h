#pragma once

#include "util/types.h"

#include "wrl/comobject.h"

#include "dxgi/dxgi_util.h"

#include "rhi_directx_util.h"

#include "descriptors/descriptor_allocation.h"

#include <memory>

namespace cera
{
    namespace renderer
    {
        class adapter;

        struct adapter_description;
    }

    namespace renderer
    {
        class CommandQueue;
        class DescriptorAllocator;
        class VertexBuffer;
        class IndexBuffer;
        class ConstantBuffer;
        class ByteAddressBuffer;
        class Texture;
        class PipelineStateObject;
        class ConstantBufferView;
        class ShaderResourceView;
        class UnorderedAccessView;
        class RootSignature;
        class resource;
        class Swapchain;

        class d3d12_device : public wrl::ComObject<ID3D12Device2>
        {
          public:
            /**
             * Reports info about the lifetime of an object or objects.
             */
            static void report_live_objects();

            /**
             * Create a new device
             */
            static std::shared_ptr<d3d12_device> create(const std::shared_ptr<adapter>& in_adapter, bool in_enable_debug_layer);

          public:
            /**
             * Get information about the selected adapter
             */
            const adapter_description& adapter_description() const;

            /**
             * Get the adapter that was used to create this device.
             */
            IDXGIAdapter* dxgi_adapter();

            /**
             * Get the adapter that was used to create this device.
             */
            const IDXGIAdapter* dxgi_adapter() const;

            /**
             * Get the d3d device.
             */
            ID3D12Device2* d3d_device();

            /**
             * Get the d3d device.
             */
            const ID3D12Device2* d3d_device() const;

            /**
             * Get a command queue. Valid types are:
             * - D3D12_COMMAND_LIST_TYPE_DIRECT : Can be used for draw, dispatch, or copy commands.
             * - D3D12_COMMAND_LIST_TYPE_COMPUTE: Can be used for dispatch or copy commands.
             * - D3D12_COMMAND_LIST_TYPE_COPY   : Can be used for copy commands.
             * By default, a D3D12_COMMAND_LIST_TYPE_DIRECT queue is returned.
             */
            CommandQueue& command_queue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);

            /**
             * Allocate a number of CPU visible descriptors.
             */
            DescriptorAllocation allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptors = 1);

            /**
             * Release stale descriptors. This should only be called with a completed frame counter.
             */
            void release_stale_descriptors();

            /**
             * Create a ConstantBuffer from a given ID3D12Resoure.
             */
            std::shared_ptr<ConstantBuffer> create_constant_buffer(wrl::com_ptr<ID3D12Resource> resource);

            /**
             * Create a ByteAddressBuffer resource.
             *
             * @param resDesc A description of the resource.
             */
            std::shared_ptr<ByteAddressBuffer> create_byte_address_buffer(memory_size bufferSize);
            std::shared_ptr<ByteAddressBuffer> create_byte_address_buffer(wrl::com_ptr<ID3D12Resource> resource);

            /**
             * Create a Texture resource.
             *
             * @param resourceDesc A description of the texture to create.
             * @param [clearVlue] Optional optimized clear value for the texture.
             * @param [textureUsage] Optional texture usage flag provides a hint about how the texture will be used.
             *
             * @returns A pointer to the created texture.
             */
            std::shared_ptr<Texture> create_texture(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr);
            std::shared_ptr<Texture> create_texture(wrl::com_ptr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);

            std::shared_ptr<IndexBuffer> create_index_buffer(size_t numIndices, DXGI_FORMAT indexFormat);
            std::shared_ptr<IndexBuffer> create_index_buffer(wrl::com_ptr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat);

            std::shared_ptr<VertexBuffer> create_vertex_buffer(size_t numVertices, size_t vertexStride);
            std::shared_ptr<VertexBuffer> create_vertex_buffer(wrl::com_ptr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride);

            std::shared_ptr<RootSignature> create_root_signature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);

            template <class TPipelineStateStream> std::shared_ptr<PipelineStateObject> create_pipeline_state_object(TPipelineStateStream& pipelineStateStream);

            std::shared_ptr<ConstantBufferView> create_constant_buffer_view(const std::shared_ptr<ConstantBuffer>& constantBuffer, size_t offset = 0);
            std::shared_ptr<ShaderResourceView> create_shader_resource_view(const std::shared_ptr<resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr);
            std::shared_ptr<UnorderedAccessView> create_unordered_access_view(const std::shared_ptr<resource>& inResource, const std::shared_ptr<resource>& inCounterResource = nullptr, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr);

            /**
             * Flush all command queues.
             */
            void flush();

            /**
             * Get the highest root signature version
             */
            D3D_ROOT_SIGNATURE_VERSION highest_root_signature_version() const;

            /**
             * Check if the requested multisample quality is supported for the given format.
             */
            DXGI_SAMPLE_DESC multisample_quality_levels(DXGI_FORMAT format, u32 numSamples = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE) const;

            /**
             * Check if tearing is supported
             */
            bool is_tearing_supported() const;

          protected:
            /**
             * The device's constructor should always be private to prevent direct construction calls
             */
            d3d12_device(std::shared_ptr<adapter> adaptor, wrl::com_ptr<ID3D12Device2> device, bool is_debug);

            virtual ~d3d12_device();

          private:
            /**
             * Execute logic to create the pipeline state object
             */
            std::shared_ptr<PipelineStateObject> do_create_pipeline_state_object(const D3D12_PIPELINE_STATE_STREAM_DESC& pipelineStateStreamDesc);

          private:
            std::shared_ptr<adapter> m_adapter;

            std::unique_ptr<CommandQueue> m_direct_command_queue;
            std::unique_ptr<CommandQueue> m_compute_command_queue;
            std::unique_ptr<CommandQueue> m_copy_command_queue;

            D3D_ROOT_SIGNATURE_VERSION m_highest_root_signature_version;

            bool m_tearing_supported;
            bool m_is_debug;

            s32 m_max_none_sampler_descriptors;
            s32 m_max_sampler_descriptors;

            std::unique_ptr<DescriptorAllocator> m_descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
        };

        template <class TPipelineStateStream> std::shared_ptr<PipelineStateObject> d3d12_device::create_pipeline_state_object(TPipelineStateStream& pipelineStateStream)
        {
            D3D12_PIPELINE_STATE_STREAM_DESC pipeline_state_stream_desc = {sizeof(TPipelineStateStream), &pipelineStateStream};

            return do_create_pipeline_state_object(pipeline_state_stream_desc);
        }
    } // namespace renderer
} // namespace cera