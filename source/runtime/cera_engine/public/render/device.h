#pragma once

#include "util/types.h"

#include "device/windows_types.h"
#include "device/windows_declarations.h"

#include "render/d3dx12_declarations.h"
#include "render/descriptor_allocation.h"

#include <memory>

namespace cera
{
    class command_queue;
    class descriptor_allocator;
    class vertex_buffer;
    class index_buffer;
    class constant_buffer;
    class byte_address_buffer;
    class texture;
    class pipeline_state_object;
    class constant_buffer_view;
    class shader_resource_view;
    class unordered_access_view;
    class root_signature;
    class resource;
    class swapchain;

    class device
    {
    public:
        /**
         * Always enable the debug layer before doing anything DX12 related so all possible errors generated while creating
         * DX12 objects are caught by the debug layer.
         */
        static bool enable_debug_layer();

        static void report_live_objects();

        static std::shared_ptr<device> create();

    public:
        /**
        * Get the adapter that was used to create this device.
        */
        IDXGIAdapter4* get_dxgi_adapter() const;
        /**
        * Get the d3d device.
        */
        ID3D12Device2* get_d3d_device() const;

        /**
         * Get a command queue. Valid types are:
         * - D3D12_COMMAND_LIST_TYPE_DIRECT : Can be used for draw, dispatch, or copy commands.
         * - D3D12_COMMAND_LIST_TYPE_COMPUTE: Can be used for dispatch or copy commands.
         * - D3D12_COMMAND_LIST_TYPE_COPY   : Can be used for copy commands.
         * By default, a D3D12_COMMAND_LIST_TYPE_DIRECT queue is returned.
         */
        command_queue& get_command_queue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;

        /**
         * Allocate a number of CPU visible descriptors.
         */
        descriptor_allocation allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptors = 1);

        /**
         * Gets the size of the handle increment for the given type of descriptor heap.
         */
        UINT get_descriptor_handle_increment_size(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

        /**
         * Create a ConstantBuffer from a given ID3D12Resoure.
         */
        std::shared_ptr<constant_buffer> create_constant_buffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource);

        /**
         * Create a ByteAddressBuffer resource.
         *
         * @param resDesc A description of the resource.
         */
        std::shared_ptr<byte_address_buffer> create_byte_address_buffer(size_t bufferSize);
        std::shared_ptr<byte_address_buffer> create_byte_address_buffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource);

        /**
         * Create a Texture resource.
         *
         * @param resourceDesc A description of the texture to create.
         * @param [clearVlue] Optional optimized clear value for the texture.
         * @param [textureUsage] Optional texture usage flag provides a hint about how the texture will be used.
         *
         * @returns A pointer to the created texture.
         */
        std::shared_ptr<texture> create_texture(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr);
        std::shared_ptr<texture> create_texture(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);

        std::shared_ptr<index_buffer> create_index_buffer(size_t numIndices, DXGI_FORMAT indexFormat);
        std::shared_ptr<index_buffer> create_index_buffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat);

        std::shared_ptr<vertex_buffer> create_vertex_buffer(size_t numVertices, size_t vertexStride);
        std::shared_ptr<vertex_buffer> create_vertex_buffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride);

        std::shared_ptr<root_signature> create_root_signature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);

        template<class pipeline_state_stream>
        std::shared_ptr<pipeline_state_object> create_pipeline_state_object(pipeline_state_stream& pipelineStateStream);

        std::shared_ptr<constant_buffer_view> create_constant_buffer_view(const std::shared_ptr<constant_buffer>& constantBuffer, size_t offset = 0);
        std::shared_ptr<shader_resource_view> create_shader_resource_view(const std::shared_ptr<resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr);
        std::shared_ptr<unordered_access_view> create_unordered_access_view(const std::shared_ptr<resource>& inResource, const std::shared_ptr<resource>& inCounterResource = nullptr, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr);

        /**
        * Flush all command queues.
        */
        void flush();

        /**
         * Release stale descriptors. This should only be called with a completed frame counter.
         */
        void release_stale_descriptors();

        /**
        * Get the highest root signature version
        */
        D3D_ROOT_SIGNATURE_VERSION get_highest_root_signature_version() const;

        /**
         * Check if the requested multisample quality is supported for the given format.
         */
        DXGI_SAMPLE_DESC get_multisample_quality_levels(DXGI_FORMAT format, UINT numSamples = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE) const;
        
        /**
        * Check if tearing is supported
        */
        bool is_tearing_supported() const;

    protected:
        /**
        * The device's constructor should always be private to prevent direct construction calls
        */
        device(wrl::ComPtr<IDXGIAdapter4> adaptor, wrl::ComPtr<ID3D12Device2> device);

        virtual ~device();

    private:
        /**
         * Execute logic to create the pipeline state object
        */
        std::shared_ptr<pipeline_state_object> do_create_pipeline_state_object( const D3D12_PIPELINE_STATE_STREAM_DESC& pipelineStateStreamDesc );

    private:
        /**
        * Make std::make_unique a friend of in order to allow it to access private constructors.
        */
        template <class _Type, class... _Types, std::enable_if_t<!std::is_array_v<_Type>, int>>
        friend std::unique_ptr<_Type> std::make_unique<_Type>(_Types&&...);

        static std::unique_ptr<device> m_instance;

        wrl::ComPtr<IDXGIAdapter4> m_dxgi_adapter;
        wrl::ComPtr<ID3D12Device2> m_d3d12_device;

        std::unique_ptr<command_queue> m_direct_command_queue;
        std::unique_ptr<command_queue> m_compute_command_queue;
        std::unique_ptr<command_queue> m_copy_command_queue;

        D3D_ROOT_SIGNATURE_VERSION m_highest_root_signature_version;

        bool m_tearing_supported;

        std::unique_ptr<descriptor_allocator> m_descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    };

    template<class pipeline_state_stream>
    std::shared_ptr<pipeline_state_object> device::create_pipeline_state_object(pipeline_state_stream& pipelineStateStream)
    {
        D3D12_PIPELINE_STATE_STREAM_DESC pipeline_state_stream_desc = { sizeof(pipeline_state_stream), &pipelineStateStream };

        return do_create_pipeline_state_object(pipeline_state_stream_desc);
    }
}