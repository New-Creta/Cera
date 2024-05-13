#pragma once

#include <memory>

namespace cera
{
    class rhi_resource;
    class rhi_byte_address_buffer;
    class rhi_texture;
    class rhi_index_buffer;
    class rhi_vertex_buffer;
    class rhi_pipeline_state_object;
    class rhi_constant_buffer_view;
    class rhi_shader_resource_view;
    class rhi_unordered_address_view;

    struct rhi_texture_desc;
    struct rhi_clear_value_desc;
    struct rhi_shader_resource_view_desc;
    struct rhi_unordered_access_view_desc;
    struct rhi_pipeline_state_object_desc;

    enum class rhi_format;

    /**
     * Interface for a rendering hardware interface (RHI), defining methods for creating various types of resources.
     */
    class rhi
    {
    public:
        /**
         * Initialize the rendering hardware interface.
         */
        virtual void initialize() = 0;

        /**
         * Perform post-initialization tasks for the rendering hardware interface.
         */
        virtual void post_initialize() = 0;

        /**
         * Shutdown the rendering hardware interface and release associated resources.
         */
        virtual void shutdown() = 0;

        /**
         * Create a ByteAddressBuffer resource.
         *
         * @param bufferSize The size of the buffer in bytes.
         * @returns A shared pointer to the created ByteAddressBuffer.
         */
        virtual std::shared_ptr<rhi_byte_address_buffer> create_byte_address_buffer(size_t bufferSize) = 0;

        /**
         * Create a Texture resource.
         *
         * @param resourceDesc A description of the texture to create.
         * @param [clearVlue] Optional optimized clear value for the texture.
         * @param [textureUsage] Optional texture usage flag provides a hint about how the texture will be used.
         *
         * @returns A pointer to the created texture.
         */
        virtual std::shared_ptr<rhi_texture> create_texture(const rhi_texture_desc& desc, const rhi_clear_value_desc& clear_value) = 0;

        /**
         * Create a Texture resource.
         *
         * @param desc A description of the texture to create.
         * @param clearValue Optional optimized clear value for the texture.
         * @param textureUsage Optional texture usage flag provides a hint about how the texture will be used.
         *
         * @returns A shared pointer to the created Texture.
         */
        virtual std::shared_ptr<rhi_index_buffer> create_index_buffer(size_t num_indices, rhi_format index_format) = 0;
        
        /**
         * Create a VertexBuffer resource.
         *
         * @param num_vertices Number of vertices in the buffer.
         * @param vertex_stride Size of each vertex in bytes.
         *
         * @returns A shared pointer to the created VertexBuffer.
         */
        virtual std::shared_ptr<rhi_vertex_buffer> create_vertex_buffer(size_t num_vertices, size_t vertex_stride) = 0;

        /**
         * Create a PipelineStateObject resource.
         *
         * @param pipeline_state_stream Description of the pipeline state.
         *
         * @returns A shared pointer to the created PipelineStateObject.
         */
        virtual std::shared_ptr<rhi_pipeline_state_object> create_pipeline_state_object(const rhi_pipeline_state_object_desc& pipeline_state_stream) = 0;

        /**
         * Create a ShaderResourceView for a resource.
         *
         * @param in_resource The resource to create the view for.
         * @param srv Description of the shader resource view.
         *
         * @returns A shared pointer to the created ShaderResourceView.
         */
        virtual std::shared_ptr<rhi_shader_resource_view> create_shader_resource_view(const std::shared_ptr<rhi_resource>& in_resource, const rhi_shader_resource_view_desc& srv) = 0;
        
        /**
         * Create an UnorderedAccessView for a resource.
         *
         * @param in_resource The resource to create the view for.
         * @param in_counter_resource The counter resource associated with the unordered access view.
         * @param uav Description of the unordered access view.
         *
         * @returns A shared pointer to the created UnorderedAccessView.
         */
        virtual std::shared_ptr<rhi_unordered_address_view> create_unordered_access_view(const std::shared_ptr<rhi_resource>& in_resource, const std::shared_ptr<rhi_resource>& in_counter_resource, const rhi_unordered_access_view_desc& uav) = 0;
    };
}